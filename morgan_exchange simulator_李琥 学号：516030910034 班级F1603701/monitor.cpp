#include <array>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <string>
#include <fstream>

using boost::asio::ip::tcp;
using namespace std;

//monitor used to record messages
int main(int argc, char* argv[])
{
	ofstream ost("monitor");
	boost::asio::io_service io_service;
	tcp::acceptor acc(io_service, tcp::endpoint(tcp::v6(), 9876));
	while (true) {
		srand(time(NULL));
		boost::system::error_code ignored;

		tcp::socket socket(io_service);
		acc.accept(socket);

		array<char, 256> input_buffer;
		size_t input_size = socket.read_some(boost::asio::buffer(input_buffer), ignored);
		string buf(input_buffer.data(), input_buffer.data() + input_size);

		ost << buf << endl;

		socket.shutdown(tcp::socket::shutdown_both, ignored);
		socket.close();

	return 0;
}