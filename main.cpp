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
char HIGHLIGHTER = '+';

enum Types {
  INT,
  FLOAT,
  STRING,
  CHAR,
  VOID
};

std::map<Types, std::string> type_map;

void type_map_init() {
  type_map[INT]    = "int";
  type_map[FLOAT]  = "float";
  type_map[STRING] = "string";
  type_map[CHAR]   = "char";
  type_map[VOID]   = "void";
}

struct Error {
  std::string filename;
  std::string highlighted;
  std::string token;
  std::string ctoken;
  std::string type;
  std::string ctype;
  size_t chars_count;
  size_t token_count;
  Error(std::string highlighted, std::string filename, std::string token, std::string ctoken, size_t chars_count, size_t token_count)
    : filename(filename), token(token), highlighted(highlighted),
      chars_count(chars_count), token_count(token_count), ctoken(ctoken) {
  }
};

std::string determine_type(std::string str) {
  Types type = VOID;
  bool fdecimal = false;
  for (int i = 0; i < str.size(); i++) {
    char c = str[i];
    if (c == '\n') continue;
    if (i == 0) {
      if (isdigit(c)) {
        type = INT;
      } else if (c == '.' && str.size() > 1) {
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
  return type_map[type];
}

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

void find_errors(std::string &orig_content, std::string &new_content, std::string ofile, std::string nfile) {
  std::vector<Error> errors;
  std::vector<std::string> orig_tokens, new_tokens;
  std::string highlighted = "";

  tokenize(orig_content, orig_tokens);
  tokenize(new_content, new_tokens);

  size_t chars_count = 1, token_count = 1;

  for (int i = 0; i < orig_tokens.size(); i++) {
    if (orig_tokens[i] != new_tokens[i]) {
      for (int j = 0; j < i; j++) {
        highlighted += orig_tokens[j];
        highlighted += " ";
      }
      highlighted += HIGHLIGHTER;
      highlighted += orig_tokens[i];
      if (orig_tokens[i][orig_tokens[i].size()-1] == '\n') {
        highlighted.erase(highlighted.size()-1);
        highlighted += HIGHLIGHTER;
        highlighted += '\n';
      } else {
        highlighted += HIGHLIGHTER;
      }
      highlighted += " ";
      for (int j = i + 1; j < orig_tokens.size(); j++) {
        highlighted += orig_tokens[j];
        highlighted += " ";
      }
      Error e(highlighted, ofile, orig_tokens[i], new_tokens[i], chars_count, token_count);
      e.ctoken = std::string(new_tokens[i]);
      e.ctype = determine_type(e.ctoken);
      e.type = determine_type(e.token);
      errors.push_back(e);
      highlighted.clear();
    }
    chars_count += orig_tokens[i].size() + 1;
    token_count += 1;
  }

  for (int i = 0; i < errors.size(); i++) {
    std::cout << "------------------\n";
    std::cout << "\nOUTPUT -> " << errors[i].highlighted;
    std::cout << "\nEXPECTED -> " << errors[i].ctoken;
    std::cout << "\nGot: " << errors[i].type << "\nExpected: " << errors[i].ctype << '\n';
    if (errors[i].type == errors[i].ctype && errors[i].type == "int") {
      std::cout << "Off by: " << stoi(errors[i].token) - stoi(errors[i].ctoken) << '\n';
    } else if (errors[i].type == errors[i].ctype && errors[i].type == "float") {
      std::cout << "Off by: " << (::atof(errors[i].token.c_str()) - ::atof(errors[i].ctoken.c_str())) << '\n';
    }
    std::cout << "Chars in: " << errors[i].chars_count << "\nToken number: " << errors[i].token_count << '\n';
  }

  if (errors.size() == 0) {
    std::cout << ofile << " and " << nfile << " match\n";
    return;
  }

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

  type_map_init();

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

    find_errors(orig_content, new_content, ofiles[i], nfiles[i]);
  }

  return 0;
}
