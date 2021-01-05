#include "instruction_mem.hpp"
#include <string.h>

using namespace std;

bool
scm::inst_mem_module::loader(char* filename) {
    SCMULATE_INFOMSG(2, "LOADING FILE %s", filename);
    ifstream file_stream;
    string line;
    file_stream.open(filename);
    unsigned int curInst = 0;

    // If file is valid
    if (file_stream.is_open()) {
      // Read line by line and get the values 
      while (std::getline(file_stream, line))
        // if line has text. Lines with no text should not be 
        // counted for offsets. Label lines are not counted for offsets
        if (line.length() != 0 ) {
          // When it is a label we store this value, it is not stored in memory
          if (instructions::isLabel(line)) {
            std::string label = instructions::getLabel(line);
            SCMULATE_INFOMSG(4, "Found label: '%s'", label.c_str());
            labels[label] = curInst; 
          } else if (!instructions::isComment(line)) {
            scm::decoded_instruction_t * inst = scm::instructions::findInstType(line); 
            if (!inst->decodeOperands(this->reg_file_m)) {
              SCMULATE_ERROR(0, "PROBLEM DECODING OPERANDS %s", filename);
              return false;
            }
            this->memory.push_back(inst);
            // Any other instruction we store it in memory
            curInst++;
          }
        }
      file_stream.close();
    } else {
      SCMULATE_ERROR(0, "PROGRAM DOES NOT EXIST %s", filename);
      return false;
    }
    return true;
}

scm::inst_mem_module::inst_mem_module(char * filename, reg_file_module * const reg_file_m):
  reg_file_m(reg_file_m) {
  SCMULATE_INFOMSG(3, "CREATING INSTRUCTION MEMORY");
  this->is_valid = true;
  // Open the file, if specified, otherwise read from stdio
  string line = "";
  if (strlen(filename) != 0) {
    this->is_valid = this->loader(filename);
  } else {
    while ((cin >> line) && line != std::string("-"))
      this->memory.push_back(scm::instructions::findInstType(line));
  }
}

void
scm::inst_mem_module::dumpMemory() {

  auto it = this->memory.rbegin();
  for (; it != this->memory.rend(); it++) 
    cout << "-" << static_cast<int>(it - memory.rbegin()) << " " << *it << endl;
}

scm::inst_mem_module::~inst_mem_module() {
  for (auto it : this->memory) {
    delete it;
  }
}
