#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

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
      chars_count(chars_count), token_count(token_count), type("") {
    type_map[INT]    = "int";
    type_map[FLOAT]  = "float";
    type_map[STRING] = "string";
    type_map[CHAR]   = "char";
    type_map[VOID]   = "void";
  }

  std::string determine_type() {
    Types type = VOID;
    bool fdecimal = false;
    for (int i = 0; i < this->token.size(); i++) {
      char c = this->token[i];
      if (i == 0) {
        if (isdigit(c)) {
          type = INT;
        } else if (c == '.') {
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
};

void print_errors(std::vector<Error> &errors, std::string ofile, std::string nfile) {
  if (errors.size() == 0) {
    std::cout << "Files: " << ofile << " and " << nfile << " match\n";
  }
}

std::vector<Error> find_errors(std::string &orig_content, std::string &new_content) {
  std::vector<Error> errors;
  return errors;
}

void usage() {
  fprintf(stderr, "usage: ./diff_finder <ofile1> <nfile1> (optional <ofile2> <nfile2> ... <ofileN> <nfileN>)");
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

    Error e("", "1234", 0, 0);
    std::cout << e.determine_type() << std::endl;
    // std::vector<Error> errors = find_errors(orig_content, new_content);
    // print_errors(errors, ofiles[i], nfiles[i]);
  }

  return 0;
}
