#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string.h>
#include <vector>

#define NOOP(x) (void(x))
#define TODO (std::cout << "TODO: line " << __LINE__ << std::endl); \
  (exit(1));

char DELIMITER = ' ';

struct Error {
  enum Types {
    INT,
    FLOAT,
    STRING,
    CHAR,
    VOID
  };
  std::map<Types, std::string> type_map;
  std::string filename;
  std::string token;
  std::string type;
  size_t chars_count;
  size_t token_count;
  Error(std::string filename, std::string token, size_t chars_count, size_t token_count)
    : filename(filename), token(token),
      chars_count(chars_count), token_count(token_count) {
    type_map[INT]    = "int";
    type_map[FLOAT]  = "float";
    type_map[STRING] = "string";
    type_map[CHAR]   = "char";
    type_map[VOID]   = "void";
    Types type = VOID;
    bool fdecimal = false;
    for (int i = 0; i < this->token.size(); i++) {
      char c = this->token[i];
      if (i == 0) {
        if (isdigit(c)) {
          type = INT;
        } else if (c == '.' && this->token.size() > 1) {
          type = FLOAT;
          fdecimal = true;
        } else {
          type = CHAR;
        }
      } else {
        if (c == '.' && (type == INT || type == VOID) && !fdecimal) {
          type = FLOAT;
          fdecimal = true;
        } else if (!isdigit(c)) {
          type = STRING;
        }
      }
    }
    this->type = type_map[type];
  }

};

// https://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c
void tokenize(std::string str, std::vector<std::string> &tokens) {
  size_t start = str.find_first_not_of(DELIMITER), end=start;

  while (start != std::string::npos){
    // Find next occurence of delimiter
    end = str.find(DELIMITER, start);
    // Push back the token found into vector
    tokens.push_back(str.substr(start, end-start));
    // Skip all occurences of the delimiter to find new start
    start = str.find_first_not_of(DELIMITER, end);
  }
}

void dump_errors(std::vector<Error> &errors, std::string ofile, std::string nfile) {
  if (errors.size() == 0) {
    std::cout << "Files: " << ofile << " and " << nfile << " match\n";
    return;
  }

  for (int i = 0; i < errors.size(); i++) {
    std::cout << errors[i].type << std::endl;
  }
}

std::vector<Error> find_errors(std::string &orig_content, std::string &new_content, std::string &filename) {
  std::vector<Error> errors;
  std::vector<std::string> orig_tokens, new_tokens;

  tokenize(orig_content, orig_tokens);
  tokenize(new_content, new_tokens);

  size_t chars_count = 0, token_count = 0;

  for (int i = 0; i < orig_tokens.size(); i++) {
    if (orig_tokens[i] != new_tokens[i]) {
      errors.push_back(Error(filename, orig_tokens[i], chars_count, token_count));
    }
    chars_count += orig_tokens[i].size();
    token_count += 1;
  }

  return errors;
}

void usage() {
  fprintf(stderr, "usage: ./diff_finder <ofile1> <nfile1> (optional <ofile2> <nfile2> ... <ofileN> <nfileN>)\n");
}

int main(int argc, char **argv) {
  *argv++;
  argc--;

  if (argc < 2 || argc % 2 != 0) {
    usage();
    exit(EXIT_FAILURE);
  }

  std::string ofiles[argc/2], nfiles[argc/2];

  for (int i = 0; *argv; i++) {
    ofiles[i] = *argv++;
    nfiles[i] = *argv++;
  }

  size_t files_sz = sizeof(ofiles) / sizeof(std::string);

  for (int i = 0; i < files_sz; i++) {
    std::ifstream orig_fp(ofiles[i]);
    std::ifstream new_fp(nfiles[i]);
    std::stringstream buffer;
    std::string orig_content, new_content;

    if (orig_fp.is_open()) {
      buffer << orig_fp.rdbuf();
      orig_content = buffer.str();
    }

    buffer.str(std::string());

    if (new_fp.is_open()) {
      buffer << new_fp.rdbuf();
      new_content = buffer.str();
    }

    std::vector<Error> errors = find_errors(orig_content, new_content, ofiles[i]);
    dump_errors(errors, ofiles[i], nfiles[i]);
  }

  return 0;
}
