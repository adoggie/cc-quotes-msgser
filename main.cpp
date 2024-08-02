#include <iostream>

#include "message.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
    std::string s = "helloworld";
    cc_quotes::Message msg;
    msg.id = "001";
    msg.name = "sam";
    msg.ts = 1111222222;
    std::copy_n( (unsigned char*)s.c_str(),s.size(),std::back_inserter(msg.data));
    msg.attrs["loc"] = "shanghai";
    msg.attrs["ip"] = "1.2.2.3";
    auto data = msg.marshall();
    std::cout<< data->size() << std::endl;
    {
        auto msg = cc_quotes::Message::parse(data->data(), data->size());
        if( msg ){
            std::cout << "message decoded ok " << std::endl;
        }
    }

    return 0;
}
