///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2014 Tavendo GmbH
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

#if 1

// http://stackoverflow.com/questions/22597948/using-boostfuture-with-then-continuations/
#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#define BOOST_THREAD_PROVIDES_FUTURE_WHEN_ALL_WHEN_ANY
#include <boost/thread/future.hpp>

struct Foo {

   boost::future<int> start() {
      return p.get_future();
   }

   void finish() {
      p.set_value(23);
   }

   boost::promise<int> p;
};


int main () {
    Foo foo;

    auto f1 = foo.start();
    auto f2 = f1.then([](boost::future<int> f) {
        std::cout << "done:" << std::endl;
        std::cout << f.get() << std::endl;
    });

    foo.finish();
    //f2.get();
}

#else

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "autobahn.hpp"

#include <boost/asio.hpp>

using namespace std;
using namespace boost;

using boost::asio::ip::tcp;

template<typename R>
  bool is_ready(future<R>& f)
  { return f.wait_for(std::chrono::seconds(0)) == future_status::ready; }



// http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/examples/cpp11_examples.html


int main () {

   try {
      asio::io_service io;

      asio::deadline_timer timer(io, posix_time::seconds(2));
      timer.async_wait(
         [](system::error_code ec) {
            if (!ec) {
               cerr << "Timeout!" << endl;
            } else {
               cerr << "Error in timer" << endl;
            }
         }
      );

#if 1
      tcp::socket socket(io);

      autobahn::session<tcp::socket,
                        tcp::socket> session(io, socket, socket);

      tcp::resolver resolver(io);
      auto endpoint_iterator = resolver.resolve({"127.0.0.1", "8080"});


      boost::asio::async_connect(socket, endpoint_iterator,

         [&](boost::system::error_code ec, tcp::resolver::iterator) {

            if (!ec) {
               cerr << "connected" << endl;
               session.start();

               auto s = session.join(string("realm1")).then([&](future<int> s) {
                  cerr << "session joined" << endl;
                  session.publish("com.myapp.topic1");

#if 1
                  auto c = session.call("com.arguments.add2", {2, 3})
                     .then([&](future<any> f) {

                     cerr << "call returned" << endl;
                     //cerr << "future valid: " << f.valid() << endl;
                     //cerr << "future ready: " << is_ready(f) << endl;

                     any r = f.get();

                     cerr << "result type: " << r.type().name() << endl;

                     uint64_t res = any_cast<uint64_t> (r);

                     cerr << "result: " << res << endl;
                  });

                  cerr << "HERE2" << endl;

                  c.get();

                  //c.wait();
#endif
               });

            } else {
               cerr << "could not connect" << endl;
            }
         }
      );
#endif


#if 1
      io.run();
#else      
      std::thread t([&io](){ io.run(); });
      t.join();
#endif

      cerr << "I/O loop ended .." << endl;
   }
   catch (std::exception& e) {
      cerr << e.what() << endl;
      return 1;
   }
   return 0;
}
#endif