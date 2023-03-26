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

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

string ReadLine() {
	string s;
	getline(cin, s);
	return s;
}

int ReadLineWithNumber() {
	int result;
	cin >> result;
	ReadLine();
	return result;
}

vector<string> SplitIntoWords(const string& text) {
	vector<string> words;
	string word;
	for (const char c : text) {
		if (c == ' ') {
			if (!word.empty()) {
				words.push_back(word);
				word.clear();
			}
		}
		else {
			word += c;
		}
	}
	if (!word.empty()) {
		words.push_back(word);
	}

	return words;
}

struct Document {
	Document() = default;

	Document(int id_s, double relevance_s, int rating_s)
		: id(id_s), relevance(relevance_s), rating(rating_s) {}

	int id = 0;
	double relevance = 0.0;
	int rating = 0;
};



enum class DocumentStatus {
	ACTUAL,
	IRRELEVANT,
	BANNED,
	REMOVED,
};

template <typename Iterator>
class Paginator {

};

class SearchServer {
public:

	template <typename StringContainer>
	explicit SearchServer(const StringContainer& stop_words)
		: stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {}

	explicit SearchServer(const string& stop_words_text)
		: SearchServer(
			SplitIntoWords(stop_words_text)) {}

	void AddDocument(int document_id, const string& document, DocumentStatus status,
		const vector<int>& ratings) {
		if (document_id < 0)
			throw invalid_argument("ID не может быть меньше 0.");
		if (documents_.count(document_id) != 0)
			throw invalid_argument("Документ с таким ID уже существет.");
		const vector<string> words = SplitIntoWordsNoStop(document);
		const double inv_word_count = 1.0 / words.size();
		for (const string& word : words) {
			CheckValidWord(word);
			word_to_document_freqs_[word][document_id] += inv_word_count;
		}
		documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
		doc_id_.push_back(document_id);
	}


	//new сприн 2 задание 6
	vector<Document> FindTopDocuments(const string& raw_query) const
	{
		return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
	}

	vector<Document> FindTopDocuments(const string& raw_query,
		DocumentStatus status) const {
		return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus status_, int rating) { return status_ == status; });
	}

	template<typename DocumentPredicate >
	vector<Document> FindTopDocuments(const string& raw_query,
		DocumentPredicate doc_predicate) const
	{
		const Query query = ParseQuery(raw_query);
		vector<Document> result = FindAllDocuments(query, doc_predicate);

		sort(result.begin(), result.end(),
			[](const Document& lhs, const Document& rhs) {
				if (abs(lhs.relevance - rhs.relevance) < EPSILON) {
					return lhs.rating > rhs.rating;
				}
				else {
					return lhs.relevance > rhs.relevance;
				}
			});
		if (result.size() > MAX_RESULT_DOCUMENT_COUNT) {
			result.resize(MAX_RESULT_DOCUMENT_COUNT);
		}
		return result;
	}

	int GetDocumentCount() const {
		return documents_.size();
	}

	tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
		int document_id) const {
		const Query query = ParseQuery(raw_query);
		//for (const string& word : query.plus_words) {
		//	if (word[0] == '-' || word[word.size() - 1] == '-') {
		//		throw invalid_argument("наличие более чем одного минуса перед словом запроса: "s + word);
		//	}
		//}
		vector<string> matched_words;
		for (const string& word : query.plus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			if (word_to_document_freqs_.at(word).count(document_id)) {
				matched_words.push_back(word);
			}
		}
		for (const string& word : query.minus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			if (word_to_document_freqs_.at(word).count(document_id)) {
				matched_words.clear();
				break;
			}
		}
		return make_tuple(matched_words, documents_.at(document_id).status);
	}

	int GetDocumentId(int index) const {
		return doc_id_.at(index);
	}

	template <typename StringContainer>
	set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
		set<string> non_empty_strings;
		for (const string& str : strings) {
			CheckValidWord(str);
			if (!str.empty()) {
				non_empty_strings.insert(str);
			}
		}
		return non_empty_strings;
	}

private:
	struct DocumentData {
		int rating;
		DocumentStatus status;
	};

	set<string> stop_words_;
	map<string, map<int, double>> word_to_document_freqs_;
	map<int, DocumentData> documents_;
	vector<int> doc_id_;

	bool IsStopWord(const string& word) const {
		return stop_words_.count(word) > 0;
	}

	vector<string> SplitIntoWordsNoStop(const string& text) const {
		vector<string> words;
		for (const string& word : SplitIntoWords(text)) {
			CheckValidWord(word);
			if (!IsStopWord(word)) {
				words.push_back(word);
			}
		}
		return words;
	}

	static int ComputeAverageRating(const vector<int>& ratings) {
		if (ratings.empty()) {
			return 0;
		}
		int rating_sum = 0;
		for (const int rating : ratings) {
			rating_sum += rating;
		}
		return rating_sum / static_cast<int>(ratings.size());
	}

	struct QueryWord {
		string data;
		bool is_minus;
		bool is_stop;
	};

	QueryWord ParseQueryWord(string text) const {
		bool is_minus = false;
		if (text[0] == '-') {
			if (text[1] == '-')
				throw invalid_argument("более одного минуса перед запросом " + text);
			if (text.size() < 2)
				throw invalid_argument("запрос заканчивается на -. отсутсвует \"минус слово\"");
			is_minus = true;
			text = text.substr(1);
		}
		return { text, is_minus, IsStopWord(text) };
	}

	struct Query {
		set<string> plus_words;
		set<string> minus_words;
	};

	Query ParseQuery(const string& text) const {
		Query query;
		for (const string& word : SplitIntoWords(text)) {
			CheckValidWord(word);
			QueryWord query_word = ParseQueryWord(word);
			if (!query_word.is_stop) {
				if (query_word.is_minus) {
					//if (query_word.data.empty()) {
					//	throw invalid_argument("word is empty");
					//}
					query.minus_words.insert(query_word.data);
				}
				else {
					query.plus_words.insert(query_word.data);
				}
			}
		}
		return query;
	}


	double ComputeWordInverseDocumentFreq(const string& word) const {
		return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
	}

	//спринт 2 задание 6
	template <typename DocumentPredicate>
	vector<Document> FindAllDocuments(const Query& query, DocumentPredicate doc_predicate) const {
		map<int, double> document_to_relevance;
		for (const string& word : query.plus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
			for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
				if (doc_predicate(document_id, documents_.at(document_id).status, documents_.at(document_id).rating)) {
					document_to_relevance[document_id] += term_freq * inverse_document_freq;
				}
			}
		}

		for (const string& word : query.minus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
				document_to_relevance.erase(document_id);
			}
		}

		vector<Document> matched_documents;
		for (const auto [document_id, relevance] : document_to_relevance) {
			matched_documents.push_back(
				{ document_id, relevance, documents_.at(document_id).rating });
		}
		return matched_documents;
	}

	vector<Document> FindAllDocuments(const Query& query, DocumentStatus status) const {
		map<int, double> document_to_relevance;
		for (const string& word : query.plus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
			for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
				if (documents_.at(document_id).status == status) {
					document_to_relevance[document_id] += term_freq * inverse_document_freq;
				}
			}
		}

		for (const string& word : query.minus_words) {
			if (word_to_document_freqs_.count(word) == 0) {
				continue;
			}
			for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
				document_to_relevance.erase(document_id);
			}
		}

		vector<Document> matched_documents;
		for (const auto [document_id, relevance] : document_to_relevance) {
			matched_documents.push_back(
				{ document_id, relevance, documents_.at(document_id).rating });
		}
		return matched_documents;
	}

	static void CheckValidWord(const string& word) {
		if (!none_of(word.begin(), word.end(), [](char c) {
			return c >= '\0' && c < ' ';
			}))
			throw invalid_argument(word + "word is invalid"s);
	}
};

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