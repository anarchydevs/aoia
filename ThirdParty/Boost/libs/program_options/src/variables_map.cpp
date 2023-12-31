// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)


#define BOOST_PROGRAM_OPTIONS_SOURCE
#include <boost/program_options/config.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>

#include <cassert>

namespace boost { namespace program_options {

    using namespace std;

    // First, performs semantic actions for 'oa'. 
    // Then, stores in 'm' all options that are defined in 'desc'. 
    BOOST_PROGRAM_OPTIONS_DECL 
    void store(const parsed_options& options, variables_map& xm,
               bool utf8)
    {       
        // TODO: what if we have different definition
        // for the same option name during different calls
        // 'store'.
        assert(options.description);
        const options_description& desc = *options.description;

        // We need to access map's operator[], not the overriden version
        // variables_map. Ehmm.. messy.
        std::map<std::string, variable_value>& m = xm;

        std::set<std::string> new_final;

        // First, convert/store all given options
        for (size_t i = 0; i < options.options.size(); ++i) {

            const string& name = options.options[i].string_key;
            // Skip positional options without name
            if (name.empty())
                continue;

            // If option has final value, skip this assignment
            if (xm.m_final.count(name))
                continue;

            // Ignore options which are not described
            //TODO: consider this.
            //if (desc.count(name) == 0)
            //    continue;

            const option_description& d = desc.find(name, false);

            variable_value& v = m[name];            
            if (v.defaulted()) {
                // Explicit assignment here erases defaulted value
                v = variable_value();
            }
            
            try {
                d.semantic()->parse(v.value(), options.options[i].value, utf8);
            }
            catch(validation_error& e)
            {
                e.set_option_name(name);
                throw;
            }
            v.m_value_semantic = d.semantic();
            
            // The option is not composing, and the value is explicitly
            // provided. Ignore values of this option for subsequent
            // calls to 'store'. We store this to a temporary set,
            // so that several assignment inside *this* 'store' call
            // are allowed.
            if (!d.semantic()->is_composing())
                new_final.insert(name);
        }
        xm.m_final.insert(new_final.begin(), new_final.end());

        
        
        // Second, apply default values.
        const vector<shared_ptr<option_description> >& all = desc.options();
        for(unsigned i = 0; i < all.size(); ++i)
        {
            const option_description& d = *all[i];
            string key = d.key("");
            // FIXME: this logic relies on knowledge of option_description
            // internals.
            // The 'key' is empty if options description contains '*'. 
            // In that 
            // case, default value makes no sense at all.
            if (key.empty())
            {
                continue;
            }
            if (m.count(key) == 0) {
            
                boost::any def;
                if (d.semantic()->apply_default(def)) {
                    m[key] = variable_value(def, true);
                    m[key].m_value_semantic = d.semantic();
                }
            }        
        }
    }

    BOOST_PROGRAM_OPTIONS_DECL 
    void store(const wparsed_options& options, variables_map& m)
    {
        store(options.utf8_encoded_options, m, true);
    }

    BOOST_PROGRAM_OPTIONS_DECL 
    void notify(variables_map& vm)
    {        
        // Lastly, run notify actions.
        for (map<string, variable_value>::iterator k = vm.begin(); 
             k != vm.end(); 
             ++k) 
        {
            k->second.m_value_semantic->notify(k->second.value());
        }               
    }

    abstract_variables_map::abstract_variables_map()
    : m_next(0)
    {}

    abstract_variables_map::
    abstract_variables_map(const abstract_variables_map* next)
    : m_next(next)
    {}

    const variable_value& 
    abstract_variables_map::operator[](const std::string& name) const
    {
        const variable_value& v = get(name);
        if (v.empty() && m_next)
            return (*m_next)[name];
        else if (v.defaulted() && m_next) {
            const variable_value& v2 = (*m_next)[name];
            if (!v2.empty() && !v2.defaulted())
                return v2;
            else return v;
        } else {
            return v;
        }
    }

    void 
    abstract_variables_map::next(abstract_variables_map* next)
    {
        m_next = next;
    }

    variables_map::variables_map()
    {}

    variables_map::variables_map(const abstract_variables_map* next)
    : abstract_variables_map(next)
    {}

    const variable_value&
    variables_map::get(const std::string& name) const
    {
        static variable_value empty;
        const_iterator i = this->find(name);
        if (i == this->end())
            return empty;
        else
            return i->second;
    }
}}
