#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>



using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

enum DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED
};

struct Document {
    int id;
    double relevance;
    int rating;
};

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

//���������� ����� ��� ��������
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

class SearchServer {
public:

    void PrintDocument(const Document& document) {

        cout << "{ document_id = "s << document.id << ", relevance = "s << document.relevance
            << ", rating = " << data_document_[document.id].rating << " }"s << endl;
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& rating_numbers) {

        ++document_count_;

        const auto words = SplitIntoWordsNoStop(document);
        double tf_value = 1. / words.size();

        data_document_.push_back({ document_id, ComputeAverageRating(rating_numbers), status });

        for (const auto& iter : words) {
            word_to_document_freqs_[iter][document_id] += tf_value;
        }


    }

    // ������ ���� �����
    void SetStopWords(const string& text) {
        if (!text.empty()) {
            for (const string& word : SplitIntoWords(text)) {
                stop_words_.insert(word);
            }
        }
    }


    // ���������� ���-5 ����� ����������� ���������� � ���� ���: {id, �������������}
    vector<Document> FindTopDocuments(const string& query, DocumentStatus status = DocumentStatus::ACTUAL) const {

        Query query_words = ParseQuery(query);

        vector<Document> id_relevance = FindAllDocuments(query_words, status);


        sort(id_relevance.begin(), id_relevance.end(), [](const Document lhs, const Document rhs) {
            return lhs.relevance > rhs.relevance;
            });

        if (id_relevance.size() > (size_t)MAX_RESULT_DOCUMENT_COUNT) {
            id_relevance.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return id_relevance;
    }

    vector<int> SetRating() {

        int count_rating;
        cin >> count_rating;
        vector<int> rating_numbers;

        for (int j(0); j < count_rating; ++j) {
            int rating_num;
            cin >> rating_num;
            rating_numbers.push_back({ rating_num });
        }
        cin.ignore();
        return rating_numbers;
    }

    int GetDocumentCount() {
        return document_count_;
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& query, int& id_document) {
        setlocale(LC_ALL, "Rus");

        Query set_words = ParseQuery(query);
        vector<string> words_query;

        for (auto word : set_words.negative) {
            if (word_to_document_freqs_.at(word).count(id_document)) {

                return { words_query, static_cast<DocumentStatus>(data_document_[id_document].status) };
            }
        }

        for (auto& word : set_words.positive) {
            if (word_to_document_freqs_.at(word).count(id_document)) {

                words_query.push_back({ word });

            }
        }
        if (!words_query.empty()) {
            sort(words_query.begin(), words_query.end());
        }

        return { words_query, static_cast<DocumentStatus>(data_document_[id_document].status) };
    }

private:

    map <string, map<int, double>> word_to_document_freqs_;
    map <int, int> document_rating_;

    set<string> stop_words_;
    int document_count_ = 0;

    struct Query {
        set<string> negative;
        set<string> positive;
    };

    struct DataDocument {
        int id;
        int rating;
        int status;
    };
    vector<DataDocument> data_document_;
    //���������� �������� ������
    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.size()) {
            int size = static_cast<int>(ratings.size());
            return (accumulate(ratings.begin(), ratings.end(), 0)) / size;
        }
        return 0;
    }

    // �������� ���� �����
    bool IsSafeWord(const string& word) const {
        if (!stop_words_.empty()) {
            return stop_words_.count(word);
        }
        else { return false; }
    }

    //�������� IDF ����� �������
    double GetIDF(const string query) const {

        return (log(document_count_ / static_cast<double>(word_to_document_freqs_.at(query).size())));
    }

    //
    int Find(int id_document) const {
        return data_document_[id_document].rating;
    }

    // ��������� text �� +����� � -�����
    Query ParseQuery(const string& text) const {

        string temp_word;
        Query query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-') {
                temp_word = word.substr(1);
                if (!(IsSafeWord(temp_word))) {
                    query_words.negative.insert(temp_word);
                }
            }
            else {
                query_words.positive.insert(word);
            }
        }

        return query_words;
    }

    // �������� ������� �� ������� ����-����
    vector<string> SplitIntoWordsNoStop(const string& text) const {

        vector<string> words;

        for (const string& word : SplitIntoWords(text)) {
            if (!(IsSafeWord(word))) {
                words.push_back(word);
            }
        }

        return words;
    }

    // ��� ������� ��������� ���������� ��� ������������� � id
    vector<Document> FindAllDocuments(const Query& query_words, const DocumentStatus status) const {

        map<int, double> document_to_relevance;

        for (const auto& query : query_words.positive) {
            if (word_to_document_freqs_.count(query)) {
                double count_idf = GetIDF(query);
                for (const auto& iter : word_to_document_freqs_.at(query)) {
                    document_to_relevance[iter.first] += iter.second * count_idf;
                }

            }
        }

        for (const auto& query : query_words.negative) {
            if (word_to_document_freqs_.count(query)) {
                for (const auto& iter : word_to_document_freqs_.at(query)) {
                    document_to_relevance.erase(iter.first);
                }
            }
        }

        vector<Document> result_doc_id_rel_rat;

        for (const auto& [key, value] : document_to_relevance) {
            if (data_document_[key].status == status) {
                result_doc_id_rel_rat.push_back({ key, value, Find(key) });
            }
        }
        return result_doc_id_rel_rat;
    }

};
/*
SearchServer CreateSearchServer() {

    SearchServer search_server;

    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();

    for (int id_document(0); id_document < document_count; ++id_document) {
        string document = ReadLine();
        vector<int> rating_numbers = search_server.SetRating();
        search_server.AddDocument(id_document, document, DocumentStatus::ACTUAL, rating_numbers);
    }

    return search_server;
}
*/
void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status) {
    cout << "{ "s
        << "document_id = "s << document_id << ", "s
        << "status = "s << static_cast<int>(status) << ", "s
        << "words ="s;
    for (const string& word : words) {
        cout << ' ' << word;
    }
    cout << "}"s << endl;
}
int main() {
    /*
    const SearchServer search_server = CreateSearchServer();
    string query = ReadLine();

    for (auto& [document_id, relevance, rating] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << ", rating = " << rating << " }"s << endl;
    }*/

    SearchServer search_server;
    search_server.SetStopWords("� � ��"s);
    search_server.AddDocument(0, "����� ��� � ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });
    /*
    cout << "ACTUAL:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("�������� ��������� ���"s)) {
        search_server.PrintDocument(document);
    }
    cout << "BANNED:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("�������� ��������� ���"s, DocumentStatus::BANNED)) {
        search_server.PrintDocument(document);
    }
    */
    const int document_count = search_server.GetDocumentCount();
    for (int i(0); i < document_count; ++i) {
        auto [words, status] = search_server.MatchDocument("�������� ���"s, i);
        PrintMatchDocumentResult(i, words, status);
    }
    return 0;
}