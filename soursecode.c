#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 10

typedef struct Node {
    char word[20], clue[100];
    int row, col, guessed;
    char dir;
    struct Node* next;
} Node;

typedef struct TrieNode {
    struct TrieNode* children[26];
    int isEnd;
} TrieNode;

Node* createNode(const char* word, const char* clue, int row, int col, char dir) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    strcpy(newNode->word, word);
    strcpy(newNode->clue, clue);
    newNode->row = row;
    newNode->col = col;
    newNode->dir = dir;
    newNode->guessed = 0;
    newNode->next = NULL;
    return newNode;
}

void insertWord(Node** head, const char* word, const char* clue, int row, int col, char dir) {
    Node* newNode = createNode(word, clue, row, col, dir);
    newNode->next = *head;
    *head = newNode;
}

TrieNode* createTrieNode() {
    TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
    for (int i = 0; i < 26; i++) node->children[i] = NULL;
    node->isEnd = 0;
    return node;
}

void insertTrie(TrieNode* root, const char* word) {
    TrieNode* temp = root;
    for (int i = 0; word[i]; i++) {
        int idx = toupper(word[i]) - 'A';
        if (!temp->children[idx]) temp->children[idx] = createTrieNode();
        temp = temp->children[idx];
    }
    temp->isEnd = 1;
}

int searchWord(TrieNode* root, const char* word) {
    TrieNode* temp = root;
    for (int i = 0; word[i]; i++) {
        int idx = toupper(word[i]) - 'A';
        if (!temp->children[idx]) return 0;
        temp = temp->children[idx];
    }
    return temp->isEnd;
}

void placeWord(char grid[MAX][MAX], const char* word, int row, int col, char dir) {
    for (int i = 0; word[i]; i++) {
        if (dir == 'H') grid[row][col + i] = toupper(word[i]);
        else if (dir == 'V') grid[row + i][col] = toupper(word[i]);
    }
}

void displayGrid(char grid[MAX][MAX]) {
    printf("\nCrossword Grid:\n");
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) printf(" %c ", grid[i][j] ? grid[i][j] : '.');
        printf("\n");
    }
}

void displayClues(Node* head) {
    printf("\nClues:\n");
    while (head) {
        printf("[%d, %d, %c] %s\n", head->row, head->col, head->dir, head->clue);
        head = head->next;
    }
}

void populateWords(Node** wordList, TrieNode** trie, char grid[MAX][MAX]) {
    insertWord(wordList, "CODE", "What programmers write?", 0, 0, 'H');
    insertWord(wordList, "BUG", "An error in a program", 2, 0, 'H');
    insertWord(wordList, "LOOP", "Used to repeat code", 1, 4, 'V');
    insertWord(wordList, "STACK", "LIFO data structure", 7, 1, 'H');
    insertWord(wordList, "ARRAY", "Collection of elements", 1, 5, 'V');

    *trie = createTrieNode();
    Node* temp = *wordList;
    while (temp) {
        insertTrie(*trie, temp->word);
        temp = temp->next;
    }
}

void saveProgress(Node* head, const char* filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error opening file to save.\n");
        return;
    }

    Node* current = head;
    while (current) {
        if (current->guessed) {
            fprintf(file, "%s\n", current->word);
        }
        current = current->next;
    }

    fclose(file);
    printf("Progress saved to '%s'.\n", filename);
}

void loadProgress(Node* head, TrieNode* trie, const char* filename, char grid[MAX][MAX]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("No saved progress found.\n");
        return;
    }

    char word[20];
    while (fgets(word, sizeof(word), file)) {
        word[strcspn(word, "\n")] = '\0';
        if (searchWord(trie, word)) {
            Node* current = head;
            while (current) {
                if (strcmp(current->word, word) == 0) {
                    current->guessed = 1;
                    placeWord(grid, word, current->row, current->col, current->dir);
                    break;
                }
                current = current->next;
            }
        }
    }

    fclose(file);
    printf("Progress loaded from '%s'.\n", filename);
}

int main() {
    char grid[MAX][MAX] = {{0}};
    Node* wordList = NULL;
    TrieNode* trie = NULL;
    populateWords(&wordList, &trie, grid);
    loadProgress(wordList, trie, "progress.txt", grid);

    char guess[20];
    int row, col;
    char dir;

    while (1) {
        displayGrid(grid);
        displayClues(wordList);

        printf("\nEnter your guess (or 'exit' to quit): ");
        scanf("%s", guess);
        if (strcmp(guess, "exit") == 0) break;

        printf("Enter starting row, col and direction (H/V): ");
        scanf("%d %d %c", &row, &col, &dir);
        dir = toupper(dir);

        Node* current = wordList;
        int found = 0;
        while (current) {
            if (!current->guessed && strcmp(current->word, guess) == 0 &&
                current->row == row && current->col == col && current->dir == dir) {
                current->guessed = 1;
                placeWord(grid, guess, row, col, dir);
                printf("Correct!\n");
                found = 1;
                break;
            }
            current = current->next;
        }

        if (!found) printf("Incorrect or already guessed!\n");
    }

    char save;
    printf("Do you want to save your progress? (y/n): ");
    scanf(" %c", &save);
    if (save == 'y' || save == 'Y') {
        saveProgress(wordList, "progress.txt");
    }

    return 0;
}
