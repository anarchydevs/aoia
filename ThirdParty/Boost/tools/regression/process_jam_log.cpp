//  process jam regression test output into XML  -----------------------------//

//  Copyright Beman Dawes 2002.  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/tools/regression for documentation.

#include "detail/tiny_xml.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/filesystem/exception.hpp"
#include "boost/filesystem/convenience.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <utility> // for make_pair
#include <ctime>
#include <cctype>   // for tolower

using std::string;
namespace xml = boost::tiny_xml;
namespace fs = boost::filesystem;

#define BOOST_NO_CPP_MAIN_SUCCESS_MESSAGE
#include <boost/test/included/prg_exec_monitor.hpp>

// options 

static bool echo = false;
static bool create_dirs = false;
static bool boost_build_v2 = false;

namespace
{
  struct test_info
  {
    string      file_path; // relative boost-root
    string      type;
    bool        always_show_run_output;
  };
  typedef std::map< string, test_info > test2info_map;  // key is test-name
  test2info_map test2info;

  fs::path boost_root;
  fs::path locate_root; // ALL_LOCATE_TARGET (or boost_root if none)

//  append_html  -------------------------------------------------------------//

  void append_html( const string & src, string & target )
  {
    // there are a few lines we want to ignore
    if ( src.find( "th target..." ) != string::npos
      || src.find( "cc1plus.exe: warning: changing search order for system directory" ) != string::npos
      || src.find( "cc1plus.exe: warning:   as it has already been specified as a non-system directory" ) != string::npos
      ) return;

    // on some platforms (e.g. tru64cxx) the following line is a real performance boost
    target.reserve(src.size() * 2 + target.size());

    for ( string::size_type pos = 0; pos < src.size(); ++pos )
    {
      if ( src[pos] == '<' ) target += "&lt;";
      else if ( src[pos] == '>' ) target += "&gt;";
      else if ( src[pos] == '&' ) target += "&amp;";
      else target += src[pos];
    }
  }

 //  timestamp  ---------------------------------------------------------------//

  string timestamp()
  {
    char run_date[128];
    std::time_t tod;
    std::time( &tod );
    std::strftime( run_date, sizeof(run_date),
      "%Y-%m-%d %X UTC", std::gmtime( &tod ) );
    return string( run_date );
  }

//  convert path separators to forward slashes  ------------------------------//

  void convert_path_separators( string & s )
  {
    for ( string::iterator itr = s.begin(); itr != s.end(); ++itr )
      if ( *itr == '\\' || *itr == '!' ) *itr = '/';
  }

//  extract a target directory path from a jam target string  ----------------//
//  s may be relative to the initial_path:
//    ..\..\..\libs\foo\build\bin\libfoo.lib\vc7\debug\runtime-link-dynamic\boo.obj
//  s may be absolute:
//    d:\myboost\libs\foo\build\bin\libfoo.lib\vc7\debug\runtime-link-dynamic\boo.obj
//  return path is always relative to the boost directory tree:
//    libs/foo/build/bin/libfs.lib/vc7/debug/runtime-link-dynamic

  string target_directory( const string & s )
  {
    string temp( s );
    convert_path_separators( temp );
    temp.erase( temp.find_last_of( "/" ) ); // remove leaf
    string::size_type pos = temp.find_last_of( " " ); // remove leading spaces
    if ( pos != string::npos ) temp.erase( 0, pos+1 );
    if ( temp[0] == '.' ) temp.erase( 0, temp.find_first_not_of( "./" ) ); 
    else temp.erase( 0, locate_root.string().size()+1 );
//std::cout << "\"" << s << "\", \"" << temp << "\"" << std::endl;
    return temp;
  }

  string::size_type target_name_end( const string & s )
  {
    string::size_type pos = s.find( ".test/" );
    if ( pos == string::npos ) pos = s.find( ".dll/" );
    if ( pos == string::npos ) pos = s.find( ".so/" );
    if ( pos == string::npos ) pos = s.find( ".lib/" );
    if ( pos == string::npos ) pos = s.find( ".pyd/" );
    if ( pos == string::npos ) pos = s.find( ".a/" );
    return pos;
  }

  string toolset( const string & s )
  {
    string::size_type pos = target_name_end( s );
    if ( pos == string::npos ) return "";
    pos = s.find( "/", pos ) + 1;
    return s.substr( pos, s.find( "/", pos ) - pos );
  }

  string test_name( const string & s )
  {
    string::size_type pos = target_name_end( s );
    if ( pos == string::npos ) return "";
    string::size_type pos_start = s.rfind( '/', pos ) + 1;
    return s.substr( pos_start,
      (s.find( ".test/" ) != string::npos
        ? pos : s.find( "/", pos )) - pos_start );
  }

  // Take a path to a target directory of test, and
  // returns library name corresponding to that path.
  string test_path_to_library_name( string const& path )
  {
    std::string result;
    string::size_type start_pos( path.find( "libs/" ) );
    if ( start_pos != string::npos )
    {
      // The path format is ...libs/functional/hash/test/something.test/....      
      // So, the part between "libs" and "test/something.test" can be considered
      // as library name. But, for some libraries tests are located too deep,
      // say numeric/ublas/test/test1 directory, and some libraries have tests
      // in several subdirectories (regex/example and regex/test). So, nested
      // directory may belong to several libraries.

      // To disambituate, it's possible to place a 'sublibs' file in
      // a directory. It means that child directories are separate libraries.
      // It's still possible to have tests in the directory that has 'sublibs'
      // file.

      std::string interesting;
      start_pos += 5;
      string::size_type end_pos( path.find( ".test/", start_pos ) );
      end_pos = path.rfind('/', end_pos);
      if (path.substr(end_pos - 5, 5) == "/test")
        interesting = path.substr( start_pos, end_pos - 5 - start_pos );
      else
        interesting = path.substr( start_pos, end_pos - start_pos );

      // Take slash separate elements until we have corresponding 'sublibs'.
      end_pos = 0;
      for(;;)
      {
        end_pos = interesting.find('/', end_pos);
        if (end_pos == string::npos) {
          result = interesting;
          break;
        }
        result = interesting.substr(0, end_pos);

        if ( fs::exists( ( boost_root / "libs" ) / result / "sublibs" ) )
        {
          end_pos = end_pos + 1;
        }
        else
          break;
      }
    }

    return result;
  }

  // Tries to find target name in the string 'msg', starting from 
  // position start.
  // If found, extract the directory name from the target name and
  // stores it in 'dir', and return the position after the target name.
  // Otherwise, returns string::npos.
  string::size_type parse_skipped_msg_aux(const string& msg,
                                          string::size_type start,
                                          string& dir)
  {
    dir.clear();
    string::size_type start_pos = msg.find( '<', start );
    if ( start_pos == string::npos ) return string::npos;
    ++start_pos;
    string::size_type end_pos = msg.find( '>', start_pos );
    dir += msg.substr( start_pos, end_pos - start_pos );
    if ( boost_build_v2 )
    {
        // The first letter is a magic value indicating
        // the type of grist.
        convert_path_separators( dir );
        dir.erase( 0, 1 );
        // We need path from root, not from 'status' dir.
        if (dir.find("../") == 0)
          dir.erase(0,3);
    }
    else
    {
      if ( dir[0] == '@' )
      {
        // new style build path, rooted build tree
        convert_path_separators( dir );
        dir.replace( 0, 1, "bin/" );
      }
      else
      {
        // old style build path, integrated build tree
        start_pos = dir.rfind( '!' );
        convert_path_separators( dir );
        dir.insert( dir.find( '/', start_pos + 1), "/bin" );
      }
    }
    return end_pos;
  }
  
  // the format of paths is really kinky, so convert to normal form
  //   first path is missing the leading "..\".
  //   first path is missing "\bin" after "status".
  //   second path is missing the leading "..\".
  //   second path is missing "\bin" after "build".
  //   second path uses "!" for some separators.
  void parse_skipped_msg( const string & msg,
    string & first_dir, string & second_dir )
  {
    string::size_type pos = parse_skipped_msg_aux(msg, 0, first_dir);
    if (pos == string::npos)
      return;
    parse_skipped_msg_aux(msg, pos, second_dir);
  }

//  test_log hides database details  -----------------------------------------//

  class test_log
    : boost::noncopyable
  {
    const string & m_target_directory;
    xml::element_ptr m_root;
  public:
    test_log( const string & target_directory,
              const string & test_name,
              const string & toolset,
              bool force_new_file )
      : m_target_directory( target_directory )
    {
      if ( !force_new_file )
      {
        fs::path pth( locate_root / target_directory / "test_log.xml" );
        fs::ifstream file( pth  );
        if ( file )   // existing file
        {
          try
          {
            m_root = xml::parse( file, pth.string() );
            return;
          }
          catch(...)
          {
            // unable to parse existing XML file, fall through
          }
        }
      }

      string library_name( test_path_to_library_name( target_directory ) );

      test_info info;
      test2info_map::iterator itr( test2info.find( library_name + "/" + test_name ) );
      if ( itr != test2info.end() )
        info = itr->second;
      
      if ( !info.file_path.empty() )
        library_name = test_path_to_library_name( info.file_path );
      
      if ( info.type.empty() )
      {
        if ( target_directory.find( ".lib/" ) != string::npos
          || target_directory.find( ".dll/" ) != string::npos 
          || target_directory.find( ".so/" ) != string::npos 
          || target_directory.find( ".dylib/" ) != string::npos 
          )
        {
          info.type = "lib";
        }
        else if ( target_directory.find( ".pyd/" ) != string::npos )
          info.type = "pyd";
      }
  
      m_root.reset( new xml::element( "test-log" ) );
      m_root->attributes.push_back(
        xml::attribute( "library", library_name ) );
      m_root->attributes.push_back(
        xml::attribute( "test-name", test_name ) );
      m_root->attributes.push_back(
        xml::attribute( "test-type", info.type ) );
      m_root->attributes.push_back(
        xml::attribute( "test-program", info.file_path ) );
      m_root->attributes.push_back(
        xml::attribute( "target-directory", target_directory ) );
      m_root->attributes.push_back(
        xml::attribute( "toolset", toolset ) );
      m_root->attributes.push_back(
        xml::attribute( "show-run-output",
          info.always_show_run_output ? "true" : "false" ) );
    }

    ~test_log()
    {
      fs::path pth( locate_root / m_target_directory / "test_log.xml" );
      if ( create_dirs && !fs::exists( pth.branch_path() ) )
          fs::create_directories( pth.branch_path() );
      fs::ofstream file( pth );
      if ( !file )
      {
        std::cout << "*****Warning - can't open output file: "
          << pth.string() << "\n";
      }
      else xml::write( *m_root, file );
    }

    const string & target_directory() const { return m_target_directory; }

    void remove_action( const string & action_name )
    // no effect if action_name not found
    {
      xml::element_list::iterator itr;
      for ( itr = m_root->elements.begin();
            itr != m_root->elements.end() && (*itr)->name != action_name;
            ++itr ) {}
      if ( itr != m_root->elements.end() ) m_root->elements.erase( itr );
    }

    void add_action( const string & action_name,
                     const string & result,
                     const string & timestamp,
                     const string & content )
    {
      remove_action( action_name );
      xml::element_ptr action( new xml::element(action_name) );
      m_root->elements.push_back( action );
      action->attributes.push_back( xml::attribute( "result", result ) );
      action->attributes.push_back( xml::attribute( "timestamp", timestamp ) );
      action->content = content;
    }
  };

//  message_manager maps input messages into test_log actions  ---------------//

  class message_manager
    : boost::noncopyable
  {
    string  m_action_name;  // !empty() implies action pending
                            // IOW, a start_message awaits stop_message
    string  m_target_directory;
    string  m_test_name;
    string  m_toolset;

    bool    m_note;  // if true, run result set to "note"
                     // set false by start_message()

    // data needed to stop further compile action after a compile failure
    // detected in the same target directory
    string  m_previous_target_directory;
    bool    m_compile_failed;

  public:
    message_manager() : m_note(false) {}
    ~message_manager() { /*assert( m_action_name.empty() );*/ }

    bool note() const { return m_note; }
    void note( bool value ) { m_note = value; }

    void start_message( const string & action_name,
                      const string & target_directory,
                      const string & test_name,
                      const string & toolset,
                      const string & prior_content )
    {
      if ( !m_action_name.empty() ) stop_message( prior_content );
      m_action_name = action_name;
      m_target_directory = target_directory;
      m_test_name = test_name;
      m_toolset = toolset;
      m_note = false;
      if ( m_previous_target_directory != target_directory )
      {
        m_previous_target_directory = target_directory;
        m_compile_failed = false;
      }
    }

    void stop_message( const string & content )
    {
      if ( m_action_name.empty() ) return;
      stop_message( m_action_name, m_target_directory,
        "succeed", timestamp(), content );
    }

    void stop_message( const string & action_name,
                     const string & target_directory,
                     const string & result,
                     const string & timestamp,
                     const string & content )
    // the only valid action_names are "compile", "link", "run", "lib"
    {
      // My understanding of the jam output is that there should never be
      // a stop_message that was not preceeded by a matching start_message.
      // That understanding is built into message_manager code.
      assert( m_action_name == action_name );
      assert( m_target_directory == target_directory );
      assert( result == "succeed" || result == "fail" );

      // if test_log.xml entry needed
      if ( !m_compile_failed
        || action_name != "compile"
        || m_previous_target_directory != target_directory )
      {
        if ( action_name == "compile"
          && result == "fail" ) m_compile_failed = true;

        test_log tl( target_directory,
          m_test_name, m_toolset, action_name == "compile" );
        tl.remove_action( "lib" ); // always clear out lib residue

        // dependency removal
        if ( action_name == "lib" )
        {
          tl.remove_action( "compile" );
          tl.remove_action( "link" );
          tl.remove_action( "run" );
        }
        else if ( action_name == "compile" )
        {
          tl.remove_action( "link" );
          tl.remove_action( "run" );
          if ( result == "fail" ) m_compile_failed = true;
        }
        else if ( action_name == "link" ) { tl.remove_action( "run" ); }

        // dependency removal won't work right with random names, so assert
        else { assert( action_name == "run" ); }

        // add the "run" stop_message action
        tl.add_action( action_name,
          result == "succeed" && note() ? "note" : result,
          timestamp, content );
      }

      m_action_name = ""; // signal no pending action
      m_previous_target_directory = target_directory;
    }
  };
}


//  main  --------------------------------------------------------------------//


int cpp_main( int argc, char ** argv )
{
  // Turn off synchronization with corresponding C standard library files. This
  // gives a significant speed improvement on platforms where the standard C++
  // streams are implemented using standard C files.
  std::ios::sync_with_stdio(false);

  if ( argc <= 1 )
    std::cout << "Usage: bjam [bjam-args] | process_jam_log [--echo] [--create-directories] [--v2] [locate-root]\n"
                 "locate-root         - the same as the bjam ALL_LOCATE_TARGET\n"
                 "                      parameter, if any. Default is boost-root.\n"
                 "create-directories  - if the directory for xml file doesn't exists - creates it.\n"
                 "                      usually used for processing logfile on different machine\n";

  boost_root = fs::initial_path();

  while ( !boost_root.empty()
    && !fs::exists( boost_root / "libs" ) )
  {
    boost_root /=  "..";
  }

  if ( boost_root.empty() )
  {
    std::cout << "must be run from within the boost-root directory tree\n";
    return 1;
  }


  if ( argc > 1 && std::strcmp( argv[1], "--echo" ) == 0 )
  {
    echo = true;
    --argc; ++argv;
  }


  if (argc > 1 && std::strcmp( argv[1], "--create-directories" ) == 0 )
  {
      create_dirs = true;
      --argc; ++argv;
  } 

  if ( argc > 1 && std::strcmp( argv[1], "--v2" ) == 0 )
  {
    boost_build_v2 = true;
    --argc; ++argv;
  }


  if (argc > 1)
  {
      locate_root = fs::path( argv[1], fs::native );
      --argc; ++argv;
  } 
  else
  {
      locate_root = boost_root;
  }

  std::cout << "boost_root: " << boost_root.string() << '\n'
            << "locate_root: " << locate_root.string() << '\n';

  message_manager mgr;

  string line;
  string content;
  bool capture_lines = false;

  std::istream* input;
  if (argc > 1)
  {
      input = new std::ifstream(argv[1]);
  }
  else
  {
      input = &std::cin;
  }

  // This loop looks at lines for certain signatures, and accordingly:
  //   * Calls start_message() to start capturing lines. (start_message() will
  //     automatically call stop_message() if needed.)
  //   * Calls stop_message() to stop capturing lines.
  //   * Capture lines if line capture on.

  while ( std::getline( *input, line ) )
  {
    if ( echo ) std::cout << line << "\n";

    // create map of test-name to test-info
    if ( line.find( "boost-test(" ) == 0 )
    {
      string::size_type pos = line.find( '"' );
      string test_name( line.substr( pos+1, line.find( '"', pos+1)-pos-1 ) );
      test_info info;
      info.always_show_run_output
        = line.find( "\"always_show_run_output\"" ) != string::npos;
      info.type = line.substr( 11, line.find( ')' )-11 );
      for (unsigned int i = 0; i!=info.type.size(); ++i )
        { info.type[i] = std::tolower( info.type[i] ); }
      pos = line.find( ':' );
      // the rest of line is missing if bjam didn't know how to make target
      if ( pos + 1 != line.size() )
      {
        info.file_path = line.substr( pos+3,
          line.find( "\"", pos+3 )-pos-3 );
        convert_path_separators( info.file_path );
        if ( info.file_path.find( "libs/libs/" ) == 0 ) info.file_path.erase( 0, 5 );
        if ( test_name.find( "/" ) == string::npos )
            test_name = "/" + test_name;
        test2info.insert( std::make_pair( test_name, info ) );
  //      std::cout << test_name << ", " << info.type << ", " << info.file_path << "\n";
      }
      else
      {
        std::cout << "*****Warning - missing test path: " << line << "\n"
          << "  (Usually occurs when bjam doesn't know how to make a target)\n";
      }
      continue;
    }

    // these actions represent both the start of a new action
    // and the end of a failed action
    else if ( line.find( "C++-action " ) != string::npos
      || line.find( "vc-C++ " ) != string::npos
      || line.find( "C-action " ) != string::npos
      || line.find( "Cc-action " ) != string::npos
      || line.find( "vc-Cc " ) != string::npos
      || line.find( "Link-action " ) != string::npos
      // archive can fail too
      || line.find( "Archive-action " ) != string::npos
      || line.find( "vc-Link " ) != string::npos 
      || line.find( ".compile.") != string::npos
      || ( line.find( ".link") != string::npos &&
           // .linkonce is present in gcc linker messages about
           // unresolved symbols. We don't have to parse those
           line.find( ".linkonce" ) == string::npos )
    )
    {
      string action( ( line.find( "Link-action " ) != string::npos
        || line.find( "vc-Link " ) != string::npos 
        || line.find( ".link") != string::npos
        || line.find( "Archive-action ") != string::npos )
        ? "link" : "compile" );
      if ( line.find( "...failed " ) != string::npos )
        mgr.stop_message( action, target_directory( line ),
          "fail", timestamp(), content );
      else
      {
        string target_dir( target_directory( line ) );
        mgr.start_message( action, target_dir,
          test_name( target_dir ), toolset( target_dir ), content );
      }
      content = "\n";
      capture_lines = true;
    }

    // these actions are only used to stop the previous action
    else if ( line.find( "-Archive" ) != string::npos
      || line.find( "MkDir" ) == 0 )
    {
      mgr.stop_message( content );
      content.clear();
      capture_lines = false;
    }

    else if ( line.find( "execute-test" ) != string::npos 
             || line.find( "capture-output" ) != string::npos )
    {
      if ( line.find( "...failed " ) != string::npos )
      {
        mgr.stop_message( "run", target_directory( line ),
          "fail", timestamp(), content );
        content = "\n";
        capture_lines = true;
      }
      else
      {
        string target_dir( target_directory( line ) );
        mgr.start_message( "run", target_dir,
          test_name( target_dir ), toolset( target_dir ), content );

        // contents of .output file for content
        capture_lines = false;
        content = "\n";
        fs::ifstream file( locate_root / target_dir
          / (test_name(target_dir) + ".output") );
        if ( file )
        {
          string ln;
          while ( std::getline( file, ln ) )
          {
            if ( ln.find( "<note>" ) != string::npos ) mgr.note( true );
            append_html( ln, content );
            content += "\n";
          }
        }
      }
    }

    // bjam indicates some prior dependency failed by a "...skipped" message
    else if ( line.find( "...skipped <" ) != string::npos && line.find( "<directory-grist>" ) == string::npos)
    {
      mgr.stop_message( content );
      content.clear();
      capture_lines = false;

      if ( line.find( " for lack of " ) != string::npos )
      {
        capture_lines = ( line.find( ".run for lack of " ) == string::npos );

        string target_dir;
        string lib_dir;

        parse_skipped_msg( line, target_dir, lib_dir );

        if ( target_dir != lib_dir ) // it's a lib problem
        {
          mgr.start_message( "lib", target_dir, 
            test_name( target_dir ), toolset( target_dir ), content );
          content = lib_dir;
          mgr.stop_message( "lib", target_dir, "fail", timestamp(), content );
          content = "\n";
        }
      }

    }

    else if ( line.find( "**passed**" ) != string::npos
      || line.find( "failed-test-file " ) != string::npos
      || line.find( "command-file-dump" ) != string::npos )
    {
      mgr.stop_message( content );
      content = "\n";
      capture_lines = true;
    }

    else if ( capture_lines ) // hang onto lines for possible later use
    {
      append_html( line, content );;
      content += "\n";
    }
  }

  mgr.stop_message( content );
  if (input != &std::cin)
      delete input;
  return 0;
}
