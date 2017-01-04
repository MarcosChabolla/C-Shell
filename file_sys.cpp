// $Id: file_sys.cpp,v 1.6 2016/07/13 00:10:55 mchaboll Exp $
//Marcos Chabolla
//ID:1437530
//mchaboll@unix.ucsc.edu
//Partner: Amit Khatri


#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash 
{
   size_t operator() (file_type type) const 
   {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) 
{
   static unordered_map<file_type,string,file_type_hash> hash 
   {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() 
{
   //Initializing a node of type directory, and gives a game
   root = make_shared<inode>(file_type::DIRECTORY_TYPE,"/");
   //Create a directory pointer,and get the contents of root
   directory_ptr root_dir = directory_ptr_of(root->get_contents());
   //Initize the directory of root
   root_dir->initialize(root,root);
   //cwd is equal to root (in this case only)
   cwd = root;
   
   
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
}

inode_ptr inode_state::get_root()
{
    //Return root inode_ptr
    return root;
}

inode_ptr inode_state::get_cwd()
{
    //return cwd inode_ptr
    return cwd;
}

void inode_state::set_cwd(inode_ptr new_cwd) 
{
  cwd = new_cwd;
}

inode_ptr inode_state::return_inode_path(const string & path) 
{
  inode_ptr parent = return_parent_path(path);
  
  directory_ptr dir_iter = directory_ptr_of(parent->get_contents());
  
  wordvec pathNames = split(path, "/");

  if(pathNames.size() <= 0) 
  {
    return parent;
  }

  parent = dir_iter->get_dirents(pathNames.at(pathNames.size()-1));

  return parent;
}

inode_ptr inode_state::return_parent_path(const string & path) 
{

  inode_ptr parent = cwd;

  if(path.at(0) == '/') 
  {
    parent = root;
  }

  directory_ptr dir_iter = directory_ptr_of(parent->get_contents());

  wordvec pathNames = split(path, "/");

 
  if(pathNames.size() <= 1) 
  {
    return parent;
  }

  for(auto & part : wordvec(pathNames.begin(), pathNames.end()-1)) 
  {
    parent = dir_iter->get_dirents(part);

    if(parent == nullptr) 
    {
      cerr << "return_parent_path: Parent is nullptr" << endl;
    }

    dir_iter = directory_ptr_of(parent->get_contents());
  }

  return parent;
}

bool inode_state::recursive_free(inode_ptr node) 
{
  string name = node->get_name();
  if(node->get_type() == file_type::PLAIN_TYPE) 
  {
    node = nullptr;
    return true;
  }
  auto dirPointer = directory_ptr_of(node->get_contents());

 
  
  for(auto & a : dirPointer->convert_to_vector()) {
    if(a.first == "." || a.first == "..") 
        continue;
    if(a.second->get_type() == file_type::DIRECTORY_TYPE) 
    {
     
        
      recursive_free(a.second);
     
      
      dirPointer->remove(a.first);
    }
  }

  for(auto & a : dirPointer->convert_to_vector()) {
    if(a.first == "." || a.first == "..") 
        continue;
    if(a.second->get_type() == file_type::DIRECTORY_TYPE) 
    {
      dirPointer->remove(a.first);
    }
  }  
  dirPointer->remove("..");
  dirPointer->remove(".");
 
  return true;
}

void inode::recursive_print(stringstream & ss,
  deque<dirent_pair> & r_stack, string name) const 
{

   directory_ptr dirPointer = nullptr;

  if(type == file_type::PLAIN_TYPE) 
  {
    print_desc(ss);
    return;
  }

  ss << name << ":" << "\n";
  print_dir(ss); 
  dirPointer = directory_ptr_of(contents);

  for(auto & pairs : dirPointer->convert_to_vector()) 
  {
    if(pairs.first != "." && pairs.first!=".." &&
    pairs.second->get_type() == file_type::DIRECTORY_TYPE) 
    {
      string a = name+"/"+pairs.first;
      r_stack.push_back({a, pairs.second});
    }
  }

  if(r_stack.size()) 
  {
    auto next = r_stack.front();
    r_stack.pop_front();
    next.second->recursive_print(ss, r_stack, next.first);
  }

}

const string& inode_state::prompt() { return prompt_; }

ostream& operator<< (ostream& out, const inode_state& state) 
{
   out << "inode_state:root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

 string inode_state::get_prompt() const 
{
  return prompt_;
}
void inode_state::set_prompt(string str) 
{
  prompt_ = str;
}

base_file_ptr inode::get_contents()
{
    return contents;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) 
{
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

inode::inode(file_type intype, string name):
   inode_nr (next_inode_nr++), type (intype), name(name)
{
   switch (type) 
   {
       case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
       case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const 
{
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

string inode::get_name() const
{
    return name;
}

file_type inode::get_type()
{
    return type;
}

void inode::print_desc(stringstream & ss, string name) const 
{

  directory_ptr dirPointer = nullptr;
  plain_file_ptr filePointer = nullptr;

  
  if(name == "") {name = get_name();};
  

  ss << "\t";
  switch(type) {
      case file_type::PLAIN_TYPE:
        dirPointer = directory_ptr_of(contents);
        ss << get_inode_nr() << "\t";
        ss << dirPointer->size() << "\t";
        ss << name;
        if(name != "." && name != "..") {
          ss << "/";
        }
        ss << "\n";
     break;

      case file_type::DIRECTORY_TYPE:
        filePointer = plain_file_ptr_of(contents);
        ss << get_inode_nr() << "\t";
        
      
        ss << name << "\n";
     break;
    }
}




directory_ptr directory_ptr_of(base_file_ptr contents)
{
      directory_ptr dirPointer = dynamic_pointer_cast<directory> (contents);
      if (dirPointer == nullptr) 
          cerr << "directory_ptr_of: nullptr" << endl;
      return dirPointer;
}

file_error::file_error (const string& what):
            runtime_error (what) 
            {
}

size_t plain_file::size() const 
{
   size_t size {0};

  if(!data.size()) 
  {
    return 0;
  }
  
  for(auto & word : data) 
  {
    size += word.length();
  }
  
  size += data.size()-1;
  DEBUGF ('i', "size = " << size);
  return size;
    
}

const wordvec& plain_file::readfile() const 
{
   DEBUGF ('i', data);
   return data;
}

void plain_file::print_file(stringstream & ss) const 
{
  for(const auto & words : data) 
  {
    ss << words << " ";
  }
}
plain_file_ptr plain_file_ptr_of (base_file_ptr ptr)
{
   plain_file_ptr plainFilePtr = dynamic_pointer_cast<plain_file> (ptr);
  
   return plainFilePtr;
}

void plain_file::writefile (const wordvec& words) 
{
   DEBUGF ('i', words);
  
   data = words;
}

void plain_file::remove (const string&) 
{
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkdir (const string&, inode_ptr) 
{
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&) 
{
   throw file_error ("is a plain file");
}

void plain_file::reset() 
{
  data.clear();
}

inode_state::~inode_state() 
{
 
  recursive_free(root);
  root = nullptr;
  cwd = nullptr;
}

size_t directory::size() const 
{
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& directory::readfile() const 
{
   throw file_error ("is a directory");
}

void directory::writefile (const wordvec&) 
{
   throw file_error ("is a directory");
}

void directory::remove (const string& filename) 
{
    
  auto temp = dirents.find(filename);
  if(temp == dirents.end()) 
  {
    cerr << "remove: directory doesnt exist" << endl;
  }
  dirents.erase(temp);
  DEBUGF ('i', filename);
}


inode_ptr directory::mkdir (const string& dirname, inode_ptr pointer)
{
   DEBUGF ('i', dirname);
   DEBUGF ('i', pointer);
   
  

  inode_ptr newNode = make_shared<inode>(file_type::DIRECTORY_TYPE, dirname);
  dirents.insert({dirname, newNode});

  auto dirPointer = directory_ptr_of(newNode->get_contents());
  dirPointer->initialize(newNode, pointer);

  return newNode; 
}

inode_ptr directory::mkfile (const string& filename) 
{
   DEBUGF ('i', filename);
   //cout<<"IN THE BACKEND !!! "<<endl;
   // if something with that name exists in the directory
  if(dirents.find(filename) != dirents.end()) 
  {
     //cout<<"IN THE BACKEND !!!2 "<<endl;
    auto inode = dirents.at(filename);

  
    // else just reset the content of the file
    auto plainFile = plain_file_ptr_of(inode->get_contents());
    plainFile->reset();
    return inode;
  }

    //cout<<"IN THE BACKEND3 !!! "<<endl;
  inode_ptr newNode = make_shared<inode>(file_type::PLAIN_TYPE, filename);
  dirents.insert({filename, newNode});

  return newNode;
   
}

void directory::initialize(inode_ptr current, inode_ptr parent)
{
  if(current == nullptr || parent == nullptr) 
  {
    cerr << "arguments is null";
  }
  dirents.insert({".", current});
  dirents.insert({"..", parent});
}

inode_ptr directory::get_dirents(string input)
{
    if(dirents.find(input) == dirents.end()) 
    {
      return nullptr;
    }
    return dirents.at(input);

}

void inode::print_dir(stringstream & stream) const 
{
 
 directory_ptr dirPointer = nullptr;

  dirPointer = directory_ptr_of(contents);
  for(auto & pairs : dirPointer->convert_to_vector()) 
  {
    pairs.second->print_desc(stream, pairs.first);
  }

}

vector<dirent_pair> directory::convert_to_vector() const
{
  vector<dirent_pair > a(dirents.size());
  
  copy(dirents.begin(), dirents.end(), a.begin());
  
  sort(a.begin()+2, a.end(),
    [](const dirent_pair & p1, const dirent_pair & p2) 
    {
    return p1.first < p2.first;
        
    }
  );
  return a;
}
