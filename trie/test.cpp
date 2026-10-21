
#include "Trie.h"
#include <iostream>

using namespace std;

int main() {
    SearchLogX::Trie trie;

    // Test insertion
    trie.insertWord("error of");
    trie.insertWord("err");
    trie.insertWord("errors");
    trie.insertWord("warning");
    trie.insertWord("warn");
    trie.insertWord("");

    // Test searchWord()
    cout << boolalpha;

    cout << "Search error: " << trie.searchWord("error") << '\n';

    cout << "Search err: "
         << trie.searchWord("err") << '\n';

    cout << "Search er: " << trie.searchWord("er") << '\n';

    cout << "Search missing: " << trie.searchWord("missing") << '\n';

    cout << "Search empty: " << trie.searchWord("") << '\n';

    // Test autocomplete()
    cout << "\nAutocomplete 'err':\n";

    for (const auto& word : trie.autocomplete("err")) {
        cout << word << '\n';
    }

    cout << "\nAutocomplete 'xyz' count: " << trie.autocomplete("xyz").size() << '\n';

    cout << "Autocomplete empty count: " << trie.autocomplete("").size() << '\n';

    return 0;
}
