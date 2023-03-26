#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <cassert>
#include <optional>
#include <iterator>
#include <deque>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;






//// -------- Вывод на экран ----------
template <typename T>
void Print(ostream& out, const T& t)
{
	bool is_first = true;
	for (const auto& element : t) {
		if (!is_first) {
			out << ", "s;
		}
		is_first = false;
		out << element;
	}
}

template <typename Element1, typename Element2>
void Print(ostream& out, const map<Element1, Element2>& container)
{

	bool is_first = true;
	for (const auto& [key, val] : container) {
		if (!is_first) {
			out << ", "s << key << ": " << val;
		}
		else
			out << key << ": "s << val;
		is_first = false;
	}
}


template<typename T>
ostream& operator<<(ostream& out, vector<T> t)
{
	out << "[";
	Print(out, t);
	out << "]";
	return out;
}

template<typename T>
ostream& operator<<(ostream& out, set<T> t)
{
	out << "{";
	Print(out, t);
	out << "}";
	return out;
}

template <typename El1, typename El2>
ostream& operator<<(ostream& out, const map<El1, El2>& container) {
	out << "{";
	Print(out, container);
	out << "}";
	return out;
}

template <typename El1, typename El2>
ostream& operator<<(ostream& out, const pair<El1, El2>& container) {
	out << container.first << ": " << container.second;
	return out;
}

//------------------------sprint 5.1 ----------------------------



template <typename Iterator>
class IteratorRange {
public:
	IteratorRange(Iterator begin, Iterator end)
		: begin_(begin), end_(end), size_(distance(begin, end)) {}

	Iterator begin() const {
		return begin_;
	}

	Iterator end() const {
		return end_;
	}

	size_t size() {
		return size_;
	}

private:
	Iterator begin_;
	Iterator end_;
	size_t size_;

};

template <typename Iterator>
class Paginator {
public:
	Paginator(Iterator begin, Iterator end, size_t page_size) {
		Iterator page_begin = begin;
		Iterator page_end = begin;
		int pages_count = ceil(double(distance(begin, end)) / page_size);
		for (int i = 0; i < pages_count; ++i) {
			if (i == pages_count - 1) {
				page_end = end;
			}
			else {
				advance(page_end, page_size);
			}
			advance(page_begin, page_size * i);
			paginator_.push_back(IteratorRange<Iterator>(page_begin, page_end));
		}

	}

	auto begin() const {
		return paginator_.begin();
	}

	auto end() const {
		return paginator_.end();
	}

	auto size() {
		return paginator_.size();
	}

private:
	vector<IteratorRange<Iterator>> paginator_;
};

template <typename Iterator>
ostream& operator<< (std::ostream& out, const IteratorRange<Iterator>& range) {
	for (Iterator it = range.begin(); it < range.end(); ++it) {
		out << *it;
	}
	return out;
}

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
	return Paginator(c.begin(), c.end(), page_size);
}

ostream& operator<<(ostream& os, const Document& doc) {
	return os << "{ document_id = "s << doc.id << ", relevance = "s << doc.relevance << ", rating = "s << doc.rating << " }"s;
}

//------------------------sprint 5.2 ----------------------------
class RequestQueue {
public:
	explicit RequestQueue(const SearchServer& search_server)
		: search_server_(search_server) {

	}
	// сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
	template <typename DocumentPredicate>
	vector<Document> AddFindRequest(const string& raw_query, DocumentPredicate document_predicate) {
		auto result = search_server_.FindTopDocuments(raw_query, document_predicate);
		if (requests_.size() >= min_in_day_)
		{
			requests_.pop_front();
			noResult_--;
		}
		if (result.empty())
			noResult_++;
		requests_.push_front({ static_cast<int>(requests_.size()) });
		return result;
	}
	vector<Document> AddFindRequest(const string& raw_query, DocumentStatus status) {
		return AddFindRequest(raw_query, [status](int document_id, DocumentStatus status_, int rating) { return status_ == status; });
	}
	vector<Document> AddFindRequest(const string& raw_query) {
		return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
	}
	int GetNoResultRequests() const {
		return noResult_;
	}
private:
	struct QueryResult {
		int result_ = 0;
	};
	const SearchServer& search_server_;
	deque<QueryResult> requests_;
	const static int min_in_day_ = 1440;
	int noResult_ = 0;
};

/*
// -------- Начало модульных тестов поисковой системы ----------
template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
	const string& func, unsigned line, const string& hint) {
	if (t != u) {
		cout << boolalpha;
		cout << file << "("s << line << "): "s << func << ": "s;
		cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
		cout << t << " != "s << u << "."s;
		if (!hint.empty()) {
			cout << " Hint: "s << hint;
		}
		cout << endl;
		abort();
	}
}

template<typename T>
void AssertImpl(const T& t, const string& t_str, const string& file,
	const string& func, unsigned line, const string& hint) {
	if (!t) {
		cout << boolalpha;
		cout << file << "("s << line << "): "s << func << ": "s;
		cout << "ASSERT_EQUAL("s << t_str << ") failed."s;
		if (!hint.empty()) {
			cout << " Hint: "s << hint;
		}
		cout << endl;
		abort();
	}
}

template <typename T>
void RunTestImpl(const T& t, const string& str_t) {
	t();
	cerr << str_t << " OK"s << endl;

}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT(a) AssertImpl((a), #a, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl(a, b, #a, #b, __FILE__, __FUNCTION__, __LINE__, hint)
#define ASSERT_HINT(a, hint) AssertImpl((a), #a, __FILE__, __FUNCTION__, __LINE__, hint)

#define RUN_TEST(func) RunTestImpl((func), #func)

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
	const int doc_id = 42;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };
	// Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
	// находит нужный документ
	{
		SearchServer server("");
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto found_docs = server.FindTopDocuments("in"s);
		ASSERT_EQUAL_HINT(found_docs.size(), 1, "Problems with adding a document"s);
		const Document& doc0 = found_docs[0];
		ASSERT_EQUAL_HINT(doc0.id, doc_id, "Problems with adding a document"s);
	}

	// Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
	// возвращает пустой результат
	{
		SearchServer server("in the"s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		ASSERT_HINT((server.FindTopDocuments("in"s).empty()), "Problems with stop words"s);
	}
}

// Тест проверяет, что поисковая система добавляет слова и убирает документы в которых есть минус слова
void TestExcludeMinusWordsFromAddedDocumentContent()
{
	{
		SearchServer server(""s);
		server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
		server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
		server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
		const auto found_docs = server.FindTopDocuments("пушистый кот ухоженный"s); // такой запрос возвращает vector<Document> размером 3
		const auto found_docs_minus = server.FindTopDocuments("пушистый кот -ухоженный"s); //такой запрос возвращает vector<Document> размером 2, т.к. есть минус слово, которое встречается в одном из документов
		ASSERT_EQUAL(found_docs.size(), 3);
		ASSERT_EQUAL(found_docs_minus.size(), 2);
	}
}


void TestMatchDocument()
{
	const int id1 = 1;
	const int id2 = 2;
	const string str1 = "белый кот и модный ошейник"s;
	const string str2 = "пушистый кот ухоженный"s;
	const vector<int> rating1 = { 8, -3 };
	const vector<int> rating2 = { 7, 2, 7 };
	const string query1 = "белый кот"s;
	const string query2 = "кот -ухоженный"s;

	{
		SearchServer server("и"s);
		server.AddDocument(id1, str1, DocumentStatus::ACTUAL, rating1);
		server.AddDocument(id2, str2, DocumentStatus::ACTUAL, rating2);
		//server.SetStopWords("и"s);
		vector<string> vectorstring = { "белый","кот" };
		DocumentStatus status = DocumentStatus::ACTUAL;
		auto [matched_words1, matched_status1] = server.MatchDocument(query1, id1);
		ASSERT_EQUAL(matched_words1, vectorstring);
		ASSERT_EQUAL(static_cast<int>(matched_status1), static_cast<int>(status));
		auto [matched_words2, matched_status2] = server.MatchDocument(query2, id2);
		ASSERT(matched_words2.empty());
	}
}

void TestSortRelevance()
{
	const int id1 = 1;
	const int id2 = 2;
	const string str1 = "белый кот и модный ошейник"s;
	const string str2 = "пушистый кот ухоженный"s;
	const vector<int> rating1 = { 1, 2, 3 };
	const vector<int> rating2 = { 3, 2, 3 };
	const string query1 = "белый кот"s;
	SearchServer server(""s);
	server.AddDocument(id1, str1, DocumentStatus::ACTUAL, rating1);
	server.AddDocument(id2, str2, DocumentStatus::ACTUAL, rating2);
	auto const result = server.FindTopDocuments(query1);
	ASSERT(result[0].relevance > result[1].relevance);
	ASSERT_EQUAL(result[1].id, id2);
}

void TestCalcRating()
{
	const string query1 = "кот"s;
	int avRating0 = 2;
	int avRating1 = -2;
	int avRating2 = 1;
	SearchServer server(""s);
	server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { -1, -2, -3 });
	server.AddDocument(2, "ухоженный кот выразительные глаза"s, DocumentStatus::ACTUAL, { 1, -1, 3, 1 });
	auto result = server.FindTopDocuments(query1);
	ASSERT_EQUAL(result[0].rating, avRating0);
	ASSERT_EQUAL(result[1].rating, avRating2);
	ASSERT_EQUAL(result[2].rating, avRating1);
}

void TestPredicate()
{
	const int id1 = 1;
	const int id2 = 2;
	const int id3 = 4;
	const string str1 = "белый кот и модный ошейник"s;
	const string str2 = "пушистый кот ухоженный"s;
	const string str3 = "белый петух"s;
	const vector<int> rating1 = { 1, 2, 3 };
	const vector<int> rating2 = { 3, 2, 3 };
	const vector<int> rating3 = { 8, 2 };
	const string query1 = "белый кот"s;
	SearchServer server(""s);
	server.AddDocument(id1, str1, DocumentStatus::ACTUAL, rating1);
	server.AddDocument(id2, str2, DocumentStatus::ACTUAL, rating2);
	server.AddDocument(id3, str3, DocumentStatus::ACTUAL, rating3);
	auto const result = server.FindTopDocuments(query1,
		[](int document_id, DocumentStatus status, int rating) {return document_id % 2 == 0; });
	ASSERT_EQUAL(result[0].id, id3);
	ASSERT_EQUAL(result[1].id, id2);
	ASSERT_EQUAL(result.size(), 2);
}

void TestStatus()
{
	const int id1 = 1;
	const int id2 = 2;
	const int id3 = 4;
	const int id4 = 7;
	const string str1 = "белый кот и модный ошейник"s;
	const string str2 = "пушистый кот ухоженный"s;
	const string str3 = "белый петух"s;
	const string str4 = "черный кот и ошейник"s;
	const vector<int> rating1 = { 1, 2, 3 };
	const vector<int> rating2 = { 3, 2, 3 };
	const vector<int> rating3 = { 8, 2 };
	const vector<int> rating4 = { 8 };
	DocumentStatus status1 = DocumentStatus::ACTUAL;
	DocumentStatus status4 = DocumentStatus::ACTUAL;
	DocumentStatus status2 = DocumentStatus::BANNED;
	DocumentStatus status3 = DocumentStatus::IRRELEVANT;
	const string query1 = "белый кот"s;
	SearchServer server(""s);
	server.AddDocument(id1, str1, status1, rating1);
	server.AddDocument(id2, str2, status2, rating2);
	server.AddDocument(id3, str3, status3, rating3);
	server.AddDocument(id4, str4, status4, rating4);
	auto result1 = server.FindTopDocuments(query1, status1);
	auto result2 = server.FindTopDocuments(query1, status2);
	auto result3 = server.FindTopDocuments(query1, status3);
	auto result4 = server.FindTopDocuments(query1);
	ASSERT_EQUAL(result1.size(), 2);
	ASSERT_EQUAL(result4.size(), 2);
	ASSERT_EQUAL(result1[0].id, id1);
	ASSERT_EQUAL(result2.size(), 1);
	ASSERT_EQUAL(result2[0].id, id2);
	ASSERT_EQUAL(result3.size(), 1);
	ASSERT_EQUAL(result3[0].id, id3);
}

void TestRelevance()
{
	const int id1 = 1;
	const int id2 = 2;
	const string str1 = "белый кот и модный ошейник"s;
	const string str2 = "пушистый кот ухоженный"s;
	const vector<int> rating1 = { 1, 2, 3 };
	const vector<int> rating2 = { 3, 2, 3 };
	const string query1 = "белый кот"s;
	SearchServer server(""s);
	server.AddDocument(id1, str1, DocumentStatus::ACTUAL, rating1);
	server.AddDocument(id2, str2, DocumentStatus::ACTUAL, rating2);
	auto result = server.FindTopDocuments(query1);
	//cout << static_cast<double> (result[0].relevance) << endl;
	ASSERT(abs(result[0].relevance - 0.138629) < EPSILON);
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
	RUN_TEST(TestExcludeMinusWordsFromAddedDocumentContent);
	RUN_TEST(TestMatchDocument);
	RUN_TEST(TestSortRelevance);
	RUN_TEST(TestCalcRating);
	RUN_TEST(TestPredicate);
	RUN_TEST(TestStatus);
	RUN_TEST(TestRelevance);
}

// --------- Окончание модульных тестов поисковой системы -----------

*/

int main() {
	SearchServer search_server("and in at"s);
	RequestQueue request_queue(search_server);
	search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
	search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
	search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, { 1, 1, 1 });
	// 1439 запросов с нулевым результатом
	for (int i = 0; i < 1439; ++i) {
		request_queue.AddFindRequest("empty request"s);
	}
	// все еще 1439 запросов с нулевым результатом
	request_queue.AddFindRequest("curly dog"s);
	// новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
	request_queue.AddFindRequest("big collar"s);
	// первый запрос удален, 1437 запросов с нулевым результатом
	request_queue.AddFindRequest("sparrow"s);
	cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
	return 0;
}