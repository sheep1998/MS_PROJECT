#include <array>
#include <iostream>
#include "stdlib.h"
#include <boost/asio.hpp>
#include<map>
#include<string>
#include<vector>
#include<sstream>
#include<set>

using namespace std;
using boost::asio::ip::tcp;

vector<string> order_detail = { "8","9","35","11","21","38","40","54","55","60","44","10" };

set<string>shares_book;

//defination for every order
class Order {
public:
	string initial_order_data;//initial data
	string order_data;
	map<string, string> mapping;
	Order(string data);
	bool check();
	string id();
	void change_OrderQty(int transaction_quantity);//total order quantity
	string buy_or_sell();//ignore tag 3 for sell short and tag 4 for sell short exempt
	int check_OrderQty();
	string toString();
	string kind_of_share();
	bool valid_share();
	string order_to_string();
	string change_status(char t150, char t39);
};

//contruction for every order
Order::Order(string data) {
	initial_order_data = data;
	order_data = data;

	stringstream ss;

	vector <string> data_store;

	//seperate the data by ';' and store them
	ss << data;
	char cell;
	string item = "";
	while (ss >> cell) {
		if (cell != ';') { item += cell; }
		else if (cell == ';') {
			data_store.push_back(item);
			item = "";
		}
	}

	//use map to connect object and variety
	for (int i = 0; i < data_store.size(); i++) {
		string item = data_store[i];
		stringstream item_stringstream;
		item_stringstream << item;
		char cell;
		string object;
		string variety;
		int left_or_right = 0;
		int last_position = item.size();
		int position = 0;
		while (item_stringstream >> cell) {
			if (left_or_right == 0 && cell != '=') {
				object += cell;
				position += 1;
			}
			else if (cell == '=') {
				left_or_right = 1;
				position += 1;
			}
			else if (left_or_right = 1) {
				if (position < last_position - 1) {
					variety += cell;
					position += 1;
				}
				else if (position == last_position - 1) {
					variety += cell;
					mapping[object] = variety;
					object = "";
					variety = "";
				}
			}
		}

	}
}

//to check whether the necessary detials are included in the new order
//to be more explict
bool Order::check() {
	int error = 0;
	for (int i = 0; i < order_detail.size(); i++) {
		if (mapping[order_detail[i]] == "")error += 1;
	}
	if (error == 0)return true;
	return false;
}

string Order::id() {
	return mapping["11"];
}

//change the total order quantity of the existing order for full fill or the new order for partial fill
void Order::change_OrderQty(int transaction_quantity) {
	stringstream ss1;
	ss1 << mapping["38"];
	int total_quantity;
	ss1 >> total_quantity;
	int odd_quantity;
	odd_quantity = total_quantity - transaction_quantity;
	stringstream ss2;
	ss2 << odd_quantity;
	string odd_quantity_s;
	ss2 >> odd_quantity_s;
	mapping["38"] = odd_quantity_s;
	order_data = order_to_string();
	//cout << initial_order_data << endl;
	//cout << order_data << endl;
}

//check whether buy or sell (ignore tag 3 for sell short and tag 4 for sell short exempt)
// 1==buy and 2==sell
string Order::buy_or_sell() {
	return mapping["54"];
}

//check the total quantity of an order
int Order::check_OrderQty() {
	string quantity_s = mapping["38"];
	stringstream ss;
	ss << quantity_s;
	int quantity;
	ss >> quantity;
	return quantity;
}

//transform order data to string
string Order::toString() {
	string now_data;
	for (int i = 0; i < order_detail.size(); i++) {
		now_data += order_detail[i];
		now_data += '=';
		now_data += mapping[order_detail[i]];
		now_data += ';';
	}
	return now_data;
}

//check the kind of share of order
string Order::kind_of_share() {
	return mapping["55"];
}

//check whether the share of order is valid
bool Order::valid_share() {
	if (shares_book.find(kind_of_share()) != shares_book.end())return true;
	else return false;
}

//transform order_detail to string
string Order::order_to_string() {
	string new_data;
	map<string, string>::iterator it;
	it = mapping.begin();
	while (it != mapping.end()) {
		new_data += it->first;
		new_data += '=';
		new_data += it->second;
		new_data += ';';
		it++;
	}
	return new_data;
}

//form the new Ack
string Order::change_status(char t150, char t39) {
	mapping["35"] = '8';
	mapping["150"] = t150;
	mapping["39"] = t39;
	return order_to_string();
}


//defination for elements in order_book
class Node {

public:
	Order order;
	Node* next;
	Node(string data) :order(data), next(NULL) {};
	Node(string data, Node* n) :order(data), next(n) {}
};

//defination for order_book
class Order_book {
private:
	Node* head;
	int size;

public:
	string name;//name of share
	Order_book(string symbol);
	~Order_book();
	void add(string data);
	int find(string price, string tag_54);
	void erase(int pos);
	string order_book_execution(Order&order);
	int check_quantity(int pos);
	bool can_find(string price, string tag_54);
	int can_find_id(string id);
	void change_existing_order(int transaction_quantity, int pos);
	string order_book_cancel(Order&order);
	string show();
};

//distruction for order_book
Order_book::~Order_book() {
	Node *now = head;
	for (int i = 0; i < size; i++) {
		Node *tmp = now->next;
		delete now;
		now = tmp;
	}
	size = 0;
	head = NULL;
	return;
}

//construction for order_book
Order_book::Order_book(string symbol) {
	name = symbol;
	size = 0;
	head = NULL;
	return;
}

//simply add a new order to order_book
void Order_book::add(string data) {
	if (size == 0)head = new Node(data);
	else {
		Node* now = head;
		for (int i = 0; i < size - 1; i++) now = now->next;
		Node* new_node = new Node(data);
		now->next = new_node;
	}
	++size;
}

//find the appropriate price for the new order
int Order_book::find(string price, string tag_54) {
	Node *now = head;
	for (int i = 0; i < size; i++) {
		if ((now->order.mapping["44"]) == price && now->order.mapping["54"] == tag_54) return i;
		now = now->next;
	}
	return -1;
}

//remove the died order in order_book
void Order_book::erase(int pos) {
	if (pos == 0) {
		Node* old_head = head;
		head = head->next;
		delete old_head;
		old_head = NULL;
	}
	else {
		Node* now = head;
		for (int i = 0; i < pos - 1; i++) now = now->next;
		Node *next_of_now = now->next;
		now->next = next_of_now->next;
		delete next_of_now;
		next_of_now = NULL;
	}
	--size;
}

//check the quantity of one existing order
int Order_book::check_quantity(int pos) {
	int quantity;
	if (pos == 0) {
		quantity = head->order.check_OrderQty();
	}
	else {
		Node* now = head;
		for (int i = 0; i < pos - 1; i++) now = now->next;
		Node *next_of_now = now->next;
		quantity = next_of_now->order.check_OrderQty();
	}
	return quantity;
}

//change the quantity of one existing order
void Order_book::change_existing_order(int transaction_quantity, int pos) {
	if (pos == 0) {
		head->order.change_OrderQty(transaction_quantity);
	}
	else {
		Node* now = head;
		for (int i = 0; i < pos - 1; i++) now = now->next;
		Node *next_of_now = now->next;
		next_of_now->order.change_OrderQty(transaction_quantity);
	}
}

//can we find appropriate order in order_book?
bool Order_book::can_find(string price, string tag_54) {
	if (find(price, tag_54) == -1)return false;
	else return true;
}

//find id
int Order_book::can_find_id(string id) {
	Node *now = head;
	for (int i = 0; i < size; i++) {
		if ((now->order.mapping["11"]) == id) return i;
		now = now->next;
	}
	return -1;
}

//show all orders
string Order_book::show() {
	string all_orders;
	Node *now = head;
	for (int i = 0; i < size; i++) {
		all_orders += now->order.order_data;
		all_orders += "|";
		now = now->next;

	}
	return all_orders;
}

//transaction in order_book
string Order_book::order_book_execution(Order&order) {
	int n = 0;
	int fill_quantity = 0;
	Order new_order = order;
	string tag_54 = new_order.buy_or_sell();
	string search_54;
	if (tag_54 == "1") search_54 = "2";
	else if (tag_54 == "2") search_54 = "1";
	string price = order.mapping["44"];
	while (new_order.check_OrderQty() > 0) {
		if (can_find(price, search_54)) {
			//full fill
			int position = find(price, search_54);
			int old_order_quantity = check_quantity(position);
			if (old_order_quantity > new_order.check_OrderQty()) {
				change_existing_order(new_order.check_OrderQty(), position);
				return order.change_status('2', '2');
			}
			else if (old_order_quantity == new_order.check_OrderQty()) {
				erase(position);
				return order.change_status('2', '2');
			}
			//partial fill
			else if (old_order_quantity < new_order.check_OrderQty()) {
				erase(position);
				new_order.change_OrderQty(old_order_quantity);
				fill_quantity += old_order_quantity;
				n += 1;
			}
		}
		else {
			add(new_order.toString());
			if (n == 0) return order.change_status('0', '0');
			else {
				stringstream ss;
				string fill;
				ss << fill_quantity;
				ss >> fill;
				string p_order;
				p_order += order.change_status('1', '1');
				p_order += "fill=";
				p_order += fill;
				p_order += ';';
				return p_order;
			};
		}
	}
}

string Order_book::order_book_cancel(Order&order) {
	if (can_find_id(order.id()) == -1)return order.change_status('9', '8');
	else {
		int position = can_find_id(order.id());
		erase(position);
		return order.change_status('4', '4');
	}
}

//defination for the whole book of kinds of order_books
class whole_book {
public:
	//instance order_book
	Order_book order_book1;
	Order_book order_book2;
	whole_book(string s1, string s2) :order_book1(s1), order_book2(s2) {};
	string execution(Order&order);
	string cancel(Order&order);
	string show();
};

//the final execution for buy or sell
string whole_book::execution(Order&order) {
	if (order.valid_share()) {
		if (order.kind_of_share() == order_book1.name) {
			return order_book1.order_book_execution(order);
		}
		else if (order.kind_of_share() == order_book2.name) {
			return order_book2.order_book_execution(order);
		}
	}
	else  return order.change_status('F', 'F');//this kind of share is not existing
}

string whole_book::cancel(Order&order) {
	if (order.valid_share()) {
		if (order.kind_of_share() == order_book1.name) {
			return order_book1.order_book_cancel(order);
		}
		else if (order.kind_of_share() == order_book2.name) {
			return order_book2.order_book_cancel(order);
		}
	}
	else  return order.change_status('9', '8');
}

string whole_book::show() {
	string order = order_book1.show();
	return order;
}


int main(int argc, char* argv[])
{
	boost::asio::io_service io_service;
	tcp::acceptor acc(io_service, tcp::endpoint(tcp::v6(), 9876));


	//initialise the book
	shares_book.insert("share1");
	shares_book.insert("share2");
	//string data1 = "8=FIX.4.2;9=0;35=D;34=0;52=2016091923:59:16.447;143=TK;1=SJTU001;11=whatever;109=EXSIMU;18=1;100=T;21=1;54=1;40=2;59=2;47=Y;15=JPY;55=9984;48=9984;22=8;167=CS;38=1500;44=6494.0000;75=20160920;204=1;10014=PS20TGB1570N;7299=sunday;7338=sunday;10197=TNL0001;10640=Y;7275=TK_ETL_JP_STP3O;7274=EMM;7337=HK;10321=TN000005265;10752=HK;10724=2;105=MINHANG;7239=100;7324=STOCK;7253=9984.tyo;10044=Y;10935=5;12=0;7235=JP;10020=JST;114=N;10147=N;7255=EMM;10863=sunday;10394=sunday;7201=TYO;7205=OM1;10=000";
	//string data = "8=FIX.4.2;9=0;35=8;34=0;52=2016091923:59:16.569;37=whatever;11=whatever;17=TSE:0920:DT110Q:A111:1;150=0;39=0;20=0;54=1;6=0;38=15500;14=0;151=15500;55=9984;48=9984;22=8;30=T;75=20160920;60=2016091923:59:16.569;9041=DT110Q00000001;7579=DT110Q;10550=P1011474324207519063;336=0;10491=Y;7205=TYO;10567=tkpxtse1:or_etl1;10484=XTKS;10492=TSE;10010=TSE;10=000;";
	string d1 = "8=FIX.4.2;9=0;35=D;11=456;21=1;38=1000;40=2;54=2;55=share1;60=2017051614:46:00;44=12.34;10=000;";
	string d2 = "8=FIX.4.2;9=0;35=D;11=asd;21=1;38=1300;40=2;54=2;55=share1;60=2017051614:53:00;44=12.57;10=000;";
	string d3 = "8=FIX.4.2;9=0;35=D;11=1q1;21=1;38=10456;40=2;54=2;55=share1;60=2017051614:54:00;44=32.76;10=000;";
	string d4 = "8=FIX.4.2;9=0;35=D;11=a45;21=1;38=1010;40=2;54=2;55=share1;60=2017051614:55:00;44=12.07;10=000;";

	whole_book book("share1", "share2");//contain kinds of shares
	book.order_book1.add(d1);
	book.order_book1.add(d2);
	book.order_book1.add(d3);
	book.order_book1.add(d4);

	while (true) {
		boost::system::error_code ignored;

		tcp::socket socket(io_service);
		acc.accept(socket);

		array<char, 256> input_buffer;
		size_t input_size = socket.read_some(boost::asio::buffer(input_buffer), ignored);
		string buf(input_buffer.data(), input_buffer.data() + input_size);

		stringstream prefix;
		prefix << buf;
		char pre;
		prefix >> pre;
		if (pre == '1') {
			prefix >> pre;
			string d;
			prefix >> d;
			Order o(d);
			string result;
			result = book.execution(o);

			boost::asio::write(socket, boost::asio::buffer(result), ignored);

			socket.shutdown(tcp::socket::shutdown_both, ignored);
			socket.close();

			boost::asio::io_service io_service;
			tcp::resolver resolver(io_service);
			tcp::resolver::query query("192.168.1.110", "9876");
			tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
			tcp::socket socket(io_service);
			boost::asio::connect(socket, endpoint_iterator);
			boost::system::error_code error;

			boost::asio::write(socket, boost::asio::buffer(result), error);
		}
		else if (pre == '2') {
			prefix >> pre;
			string d;
			prefix >> d;
			Order o(d);
			string result;
			result = book.cancel(o);

			boost::asio::write(socket, boost::asio::buffer(result), ignored);

			socket.shutdown(tcp::socket::shutdown_both, ignored);
			socket.close();

			boost::asio::io_service io_service;
			tcp::resolver resolver(io_service);
			tcp::resolver::query query("192.168.1.110", "9876");
			tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
			tcp::socket socket(io_service);
			boost::asio::connect(socket, endpoint_iterator);
			boost::system::error_code error;

			boost::asio::write(socket, boost::asio::buffer(result), error);
		}
		else if (pre == '3') {
			prefix >> pre;
			string result;
			result = book.show();

			boost::asio::write(socket, boost::asio::buffer(result), ignored);

			socket.shutdown(tcp::socket::shutdown_both, ignored);
			socket.close();

			boost::asio::io_service io_service;
			tcp::resolver resolver(io_service);
			tcp::resolver::query query("192.168.1.110", "9876");
			tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
			tcp::socket socket(io_service);
			boost::asio::connect(socket, endpoint_iterator);
			boost::system::error_code error;

			boost::asio::write(socket, boost::asio::buffer(result), error);
		}
	}
}