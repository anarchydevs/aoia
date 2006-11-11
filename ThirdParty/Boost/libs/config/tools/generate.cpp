//  (C) Copyright John Maddock 2004. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//
// This progam scans for *.ipp files in the libs/config/test
// directory and then generates the *.cpp test files from them
// along with config_test.cpp and a Jamfile.
//

#include <boost/regex.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/test/included/prg_exec_monitor.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>

namespace fs = boost::filesystem;

fs::path config_path;

std::string copyright(
"//  Copyright John Maddock 2002-4.\n"
"//  Use, modification and distribution are subject to the \n"
"//  Boost Software License, Version 1.0. (See accompanying file \n"
"//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)\n"
"\n"
"//  See http://www.boost.org/libs/config for the most recent version.");

std::stringstream config_test1;
std::stringstream config_test1a;
std::stringstream config_test2;
std::stringstream jamfile;

void write_config_test()
{
   fs::ofstream ofs(config_path / "config_test.cpp");
   time_t t = std::time(0);
   ofs << "//  This file was automatically generated on " << std::ctime(&t);
   ofs << "//  by libs/config/tools/generate.cpp\n" << copyright << std::endl;
   ofs << "// Test file for config setup\n"
      "// This file should compile, if it does not then\n"
      "// one or more macros need to be defined.\n"
      "// see boost_*.ipp for more details\n\n"
      "// Do not edit this file, it was generated automatically by\n\n"
      "#include <boost/config.hpp>\n#include <iostream>\n#include \"test.hpp\"\n\n"
      "int error_count = 0;\n\n";
   ofs << config_test1.str() << std::endl;
   ofs << config_test1a.str() << std::endl;
   ofs << "int main( int, char *[] )\n{\n" << config_test2.str() << "   return error_count;\n}\n\n";
}

void write_jamfile()
{
   fs::ofstream ofs(config_path / "Jamfile");
   time_t t = std::time(0);
   ofs << "#\n# Regression test Jamfile for boost configuration setup.\n# *** DO NOT EDIT THIS FILE BY HAND ***\n"
      "# This file was automatically generated on " << std::ctime(&t);
   ofs << "#  by libs/config/tools/generate.cpp\n"
      "# Copyright John Maddock.\n"
      "#\n# If you need to alter build preferences then set them in\n"
      "# the template defined in options.jam.\n#\n"
      "subproject libs/config/test ;\n"
      "# bring in the rules for testing\n"
      "import testing ./options ;\n\n"
      "run config_info.cpp <template>config_options ;\n"
      "run config_test.cpp <template>config_options ;\n"
      "run limits_test.cpp <template>config_test_options ;\n"
      "run abi/abi_test.cpp abi/main.cpp <template>config_options ;\n\n";
   ofs << jamfile.str() << std::endl;
}

void write_test_file(const fs::path& file, 
                     const std::string& macro_name, 
                     const std::string& namespace_name, 
                     const std::string& header_file,
                     bool positive_test, 
                     bool expect_success)
{
   if(!fs::exists(file))
   {
      std::cout << "Writing test file " << file.native_directory_string() << std::endl;

      fs::ofstream ofs(file);
      std::time_t t = std::time(0);
      ofs << "//  This file was automatically generated on " << std::ctime(&t);
      ofs << "//  by libs/config/tools/generate.cpp\n" << copyright << std::endl;
      ofs << "\n// Test file for macro " << macro_name << std::endl;

      if(expect_success)
      {
         ofs << "// This file should compile, if it does not then\n"
            "// " << macro_name << " should ";
         if(positive_test)
            ofs << "not ";
         ofs << "be defined.\n";
      }
      else
      {
         ofs << "// This file should not compile, if it does then\n"
            "// " << macro_name << " should ";
         if(!positive_test)
            ofs << "not ";
         ofs << "be defined.\n";
      }
      ofs << "// See file " << header_file << " for details\n\n";

      ofs << "// Must not have BOOST_ASSERT_CONFIG set; it defeats\n"
         "// the objective of this file:\n"
         "#ifdef BOOST_ASSERT_CONFIG\n"
         "#  undef BOOST_ASSERT_CONFIG\n"
         "#endif\n\n";

      ofs << "#include <boost/config.hpp>\n"
         "#include \"test.hpp\"\n\n"
         "#if";
      if(positive_test != expect_success)
         ofs << "n";
      ofs << "def " << macro_name << 
         "\n#include \"" << header_file << 
         "\"\n#else\n";
      if(expect_success)
         ofs << "namespace " << namespace_name << " = empty_boost;\n";
      else
         ofs << "#error \"this file should not compile\"\n";
      ofs << "#endif\n\n";

      ofs << "int main( int, char *[] )\n{\n   return " << namespace_name << "::test();\n}\n\n";  
   }
   else
   {
      std::cout << "Skipping existing test file " << file.native_directory_string() << std::endl;
   }
}

void process_ipp_file(const fs::path& file, bool positive_test)
{
   std::cout << "Info: Scanning file: " << file.native_directory_string() << std::endl;

   // our variables:
   std::string file_text;
   std::string macro_name;
   std::string namespace_name;
   fs::path positive_file;
   fs::path negative_file;

   // load the file into memory so we can scan it:
   fs::ifstream ifs(file);
   std::copy(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>(), std::back_inserter(file_text));
   ifs.close();
   // scan for the macro name:
   boost::regex macro_regex("//\\s*MACRO\\s*:\\s*(\\w+)");
   boost::smatch macro_match;
   if(boost::regex_search(file_text, macro_match, macro_regex))
   {
      macro_name = macro_match[1];
      namespace_name = boost::regex_replace(file_text, macro_regex, "\\L$1", boost::format_first_only | boost::format_no_copy);
   }
   if(macro_name.empty())
   {
      std::cout << "Error: no macro definition found in " << file.native_directory_string();
   }
   else
   {
      std::cout << "Info: Macroname: " << macro_name << std::endl;
   }

   // get the output filesnames:
   boost::regex file_regex("boost_([^.]+)\\.ipp");
   positive_file = file.branch_path() / boost::regex_replace(file.leaf(), file_regex, "$1_pass.cpp");
   negative_file = file.branch_path() / boost::regex_replace(file.leaf(), file_regex, "$1_fail.cpp");
   write_test_file(positive_file, macro_name, namespace_name, file.leaf(), positive_test, true);
   write_test_file(negative_file, macro_name, namespace_name, file.leaf(), positive_test, false);
   
   // always create config_test data,
   // positive and negative tests go to separate streams, because for some
   // reason some compilers choke unless we put them in a particular order...
   std::ostream* pout = positive_test ? &config_test1a : &config_test1;
   *pout << "#if";
   if(!positive_test)
      *pout << "n";
   *pout << "def " << macro_name 
      << "\n#include \"" << file.leaf() << "\"\n#else\nnamespace "
      << namespace_name << " = empty_boost;\n#endif\n";

   config_test2 << "   if(0 != " << namespace_name << "::test())\n"
      "   {\n"
      "      std::cerr << \"Failed test for " << macro_name << " at: \" << __FILE__ << \":\" << __LINE__ << std::endl;\n"
      "      ++error_count;\n"
      "   }\n";

   // always generate the jamfile data:
   jamfile << "test-suite \"" << macro_name << "\" : \n"
      "[ run " << positive_file.leaf() << " <template>config_options ]\n"
      "[ compile-fail " << negative_file.leaf() << " <template>config_options ] ;\n";

}

int cpp_main(int argc, char* argv[])
{
   //
   // get the boost path to begin with:
   //
   if(argc > 1)
   {
      fs::path p(argv[1], fs::native);
      config_path = p / "libs" / "config" / "test" ;
   }
   else
   {
      // try __FILE__:
      fs::path p(__FILE__, fs::native);
      config_path = p.branch_path().branch_path() / "test";
   }
   std::cout << "Info: Boost.Config test path set as: " << config_path.native_directory_string() << std::endl;

   // enumerate *.ipp files:
   boost::regex ipp_mask("boost_(?:(has)|no).*\\.ipp");
   boost::smatch ipp_match;
   fs::directory_iterator i(config_path), j;
   while(i != j)
   {
      if(boost::regex_match(i->leaf(), ipp_match, ipp_mask))
      {
         process_ipp_file(*i, ipp_match[1].matched);
      }
      ++i;
   }
   write_config_test();
   write_jamfile();
   return 0;
}