#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

struct Document {
    int id;
    double relevance;
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

//возвращаем слово без пробелов
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

    void AddDocument(int document_id, const string& document) {

        ++document_count_;

        const auto words = SplitIntoWordsNoStop(document);
        size_t word_size = words.size();

        for (const auto& iter : words) {
            word_to_document_freqs_[iter][document_id] += 1. / word_size;

        }
    }

    // парсит стоп слова
    void SetStopWords(const string& text) {

        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    // Возвращает топ-5 самых релевантных документов в виде пар: {id, релевантность}
    vector<Document> FindTopDocuments(const string& query) const {

        Query query_words = ParseQuery(query);

        vector<Document> id_relevance = FindAllDocuments(query_words);

        sort(id_relevance.begin(), id_relevance.end(), [](const Document lhs, const Document rhs) {
            return lhs.relevance > rhs.relevance;
            });

        if (id_relevance.size() > (size_t)MAX_RESULT_DOCUMENT_COUNT) {
            id_relevance.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return id_relevance;
    }

private:

    map <string, map<int, double>> word_to_document_freqs_;
    set<string> stop_words_;
    int document_count_ = 0;

    struct Query {
        set<string> negative;
        set<string> positive;
    };

    // проверка стоп слова
    bool IsSafeWord(const string& word) const {
        return stop_words_.count(word);
    }

    // Разбирает text на +слова и -слова
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

    // проверка запроса на наличие стоп-слов
    vector<string> SplitIntoWordsNoStop(const string& text) const {

        vector<string> words;

        for (const string& word : SplitIntoWords(text)) {
            if (!(IsSafeWord(word))) {
                words.push_back(word);
            }
        }

        return words;
    }

    // Для каждого документа возвращает его релевантность и id
    vector<Document> FindAllDocuments(const Query& query_words) const {

        map<int, double> document_to_relevance;

        for (const auto& query : query_words.positive) {
            if (word_to_document_freqs_.count(query)) {
                for (const auto& iter : word_to_document_freqs_.at(query)) {
                    document_to_relevance[iter.first] += iter.second * log((document_count_)
                        / static_cast<double>(word_to_document_freqs_.at(query).size()));
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

        vector<Document> result_doc_id_rel;

        for (const auto& [key, value] : document_to_relevance) {
            result_doc_id_rel.push_back({ key, value });
        }
        return result_doc_id_rel;
    }

};

SearchServer CreateSearchServer() {

    SearchServer search_server;

    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int id_document(0); id_document < document_count; ++id_document) {
        search_server.AddDocument(id_document, ReadLine());
    }

    return search_server;
}

int main() {

    const SearchServer search_server = CreateSearchServer();
    string query = ReadLine();

    for (auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << " }"s << endl;
    }
}