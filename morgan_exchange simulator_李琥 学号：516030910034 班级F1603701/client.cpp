#include <array>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include<string>
#include<Windows.h>
#include<stdio.h>
#include<sstream>
#include<vector>
#include<map>
#include<set>
using boost::asio::ip::tcp;
using namespace std;

set<string>shares_book;

vector<string> order_detail = { "8","9","35","11","21","38","40","54","55","60","44","10" };

class Order {
public:
	string initial_order_data;//initial data
	string order_data;
	map<string, string> mapping;
	Order(string data);
	void change_OrderQty(int transaction_quantity);//total order quantity
	string buy_or_sell();//ignore tag 3 for sell short and tag 4 for sell short exempt
	int check_OrderQty();
	string toString();
	string kind_of_share();
	bool valid_share();
	string order_to_string();
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
	//string order_book_execution(Order&order);
	int check_quantity(int pos);
	bool can_find(string price, string tag_54);
	void change_existing_order(int transaction_quantity, int pos);
	int findid(string id);
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

int Order_book::findid(string id) {
	Node *now = head;
	for (int i = 0; i < size; i++) {
		if ((now->order.mapping["11"]) == id) return i;
		now = now->next;
	}
	return -1;
}

string current_order;

//to born a date
string date() {
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	string date;
	stringstream s1;
	s1 << sys.wYear;
	string year;
	s1 >> year;
	date += year;
	stringstream s2;
	s2 << sys.wMonth;
	string month;
	s2 >> month;
	if (month.size() == 1)date += '0';
	date += month;
	stringstream s3;
	s3 << sys.wDay;
	string day;
	s3 >> day;
	if (day.size() == 1)date += '0';
	date += day;
	stringstream s4;
	s4 << sys.wHour;
	string hour;
	s4 >> hour;
	if (hour.size() == 1)date += '0';
	date += hour;
	date += ':';
	stringstream s5;
	s5 << sys.wMinute;
	string minute;
	s5 >> minute;
	if (minute.size() == 1)date += '0';
	date += minute;
	date += ':';
	stringstream s6;
	s6 << sys.wSecond;
	string second;
	s6 >> second;
	if (second.size() == 1)date += '0';
	date += second;
	date += '.';
	stringstream s7;
	s7 << sys.wMilliseconds;
	string wsecond;
	s7 >> wsecond;
	date += wsecond;
	return date;
}

//to born an order
void bs_order() {
	string order;
	order += "8=FIX.4.2;9=0;35=D;21=1;";
	cout << "input the symbol of the share:" << endl;//change
	string symbol;
	cin >> symbol;
	order += "55=";
	order += symbol;
	order += ';';
	cout << "buy or sell?" << endl;
	cout << "1=buy;2=sell" << endl;
	string buy_or_sell;
	cin >> buy_or_sell;
	order += "54=";
	order += buy_or_sell;
	order += ';';
	cout << "what's the ordertype?" << endl;
	cout << "1=market;2=limit" << endl;
	string ordtype;
	cin >> ordtype;
	order += "40=";
	order += ordtype;
	order += ';';
	cout << "what's the total order quantity?(1~999999)" << endl;
	string orderqty;
	cin >> orderqty;
	order += "38=";
	order += orderqty;
	order += ';';
	cout << "what's price?" << endl;
	string price;
	cin >> price;
	order += "44=";
	order += price;
	order += ';';
	cout << "input 1 to send the requirement or 2 to cancel this operation:" << endl;
	int whether;
	cin >> whether;
	if (whether == 2)return;
	order += "11=";
	string da = date();
	order += da;
	order += ";60=";
	order += da;
	order += ';';
	order += "10=000;";
	current_order = "1;" + order;
}

//to cancel an order
void cancel_order() {
	string order;
	order += "8=FIX.4.2;9=0;35=F;21=1;";
	cout << "input the order id of the existing order:" << endl;
	string id;
	cin >> id;
	order += "11=";
	order += id;
	order += ';';
	order += "38=0;40=0;54=0;55=0;60=";
	order += date();
	order += ";44=0;10=000;";
	current_order = "2;" + order;
}

// aquire for data of all orders
void inquire() {
	current_order = "3;";
}

void show_one(string data) {
	Order o(data);
	cout << "share name:   " << o.mapping["55"] << endl;
	cout << "order type:   ";
	string ordtype = o.mapping["40"];
	if (ordtype == "1") cout << "market" << endl;
	else if (ordtype == "2")cout << "limit" << endl;
	else cout << "pegged" << endl;
	cout << "Symbol(id):   " << o.mapping["11"] << endl;
	cout << "Side:   ";
	string side = o.mapping["54"];
	if (side == "1")cout << "buy" << endl;
	else if (side == "2")cout << "sell" << endl;
	else if (side == "3")cout << "sell short" << endl;
	else cout << "sell short exempt" << endl;
	cout << "total order quantity:   " << o.mapping["38"] << endl;
	cout << "-------------------------------------------------------------------------" << endl;
}

void isSuccessful(string data) {
	Order o(data);
	if (o.mapping["39"] == "4")cout << "successful!" << endl;
	else cout << "SORRY, you cannot cancel it!(Please check ID and try again)" << endl;
}

//to show all orders
void showw(string datas) {
	stringstream ss;
	ss << datas;
	char s;
	string data;
	while (ss >> s) {
		if (s != '|')data += s;
		else {
			show_one(data);
			data = "";
		}
	}
}

int main(int argc, char* argv[])
{
	while (true) {
		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query("192.168.1.247", "9876");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoint_iterator);
		boost::system::error_code error;

		cout << "1: new order" << endl;
		cout << "2:cancel order" << endl;
		cout << "3.inquire order" << endl;
		cout << "4.input q to quit" << endl;
		char choice;
		cin >> choice;
		if (choice == '1') {
			bs_order();
			boost::asio::write(socket, boost::asio::buffer(current_order), error);
			std::array<char, 256> input_buffer;
			std::size_t rsize = socket.read_some(
				boost::asio::buffer(input_buffer), error);

			std::string buf(input_buffer.begin(), input_buffer.end());
			show_one(buf);
		}
		else if (choice == '2') {
			cancel_order();
			boost::asio::write(socket, boost::asio::buffer(current_order), error);
			std::array<char, 256> input_buffer;
			std::size_t rsize = socket.read_some(
				boost::asio::buffer(input_buffer), error);

			std::string buf(input_buffer.begin(), input_buffer.end());
			isSuccessful(buf);
		}
		else if (choice == '3') {
			inquire();
			boost::asio::write(socket, boost::asio::buffer(current_order), error);
			std::array<char, 256> input_buffer;
			std::size_t rsize = socket.read_some(
				boost::asio::buffer(input_buffer), error);

			std::string buf(input_buffer.begin(), input_buffer.end());
			showw(buf);
		}
		else if (choice == 'q') {
			break;
		}
		else cout << "choose again!" << endl;
		cout << "-------------------------------------------------------------" << endl;
	}
	return 0;
}