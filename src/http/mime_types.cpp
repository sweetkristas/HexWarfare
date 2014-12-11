//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <map>

#include "mime_types.hpp"

namespace http 
{
    namespace server 
    {
        namespace mime_types 
        {
            namespace 
            {
                const std::string default_mapping_type = "text/plain";
            
                typedef std::map<std::string,std::string> mapping_t;
                const mapping_t& get_mapping()
                {
                    static mapping_t res;
                    if(res.empty()) {
                        res["gif"] = "image/gif";
                        res["htm"] = "text/html";
                        res["html"] = "text/html";
                        res["jpg"] = "image/jpeg";
                        res["png"] = "image/png";
                    }
                    return res;
                }
            }
            
            std::string extension_to_type(const std::string& extension)
            {
                auto it = get_mapping().find(extension);
                if(it != get_mapping().end()) {
                    return it->second;
                }
                return default_mapping_type;
            }
        }
    }
}
