// $Id: commands.cpp,v 1.17 2016/07/13 00:10:55 mchaboll Exp $
//Marcos Chabolla
//ID:1437530
//mchaboll@unix.ucsc.edu
//Partner: Amit Khatri


#include "commands.h"
#include "debug.h"

command_hash cmd_hash 
{
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
};

command_fn find_command_fn (const string& cmd) {
   
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

void fn_cat (inode_state& state, const wordvec& words)
{
    DEBUGF ('c', state);
    DEBUGF ('c', words);
  
    if(words.size()<=1)
    {
      cerr<<"cat: missing file argument"<<endl;
    }
    
  
   for(auto & func : wordvec(words.begin()+1, words.end())) 
   {
      inode_ptr inode = state.return_parent_path(func);
     
      auto file_ptr = plain_file_ptr_of(inode->get_contents());
      stringstream stream; stream << "";
      file_ptr->print_file(stream);

      std::cout << stream.str() << "\n";
   }
}

void fn_cd (inode_state& state, const wordvec& words)
{
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
    inode_ptr inode = nullptr;
    
    if(words.size()>2)
    {
       cerr<<"cd: Too many arguments!"<<endl;
    }

      if(words.size() != 2)
      {
         inode = state.return_parent_path("/");
      }
      else
      {
          inode = state.return_parent_path(words.at(1));
      }
    
   state.set_cwd(inode);
}

void fn_echo (inode_state& state, const wordvec& words)
{
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if(words.size() <= 1)
   {
       cout << "\n";
       return;
    }
   
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}

bool is_number(const string & str) 
{
    
   for(auto x : str) 
   {
      if(!isdigit(x)) 
      {
         return false;
      }
   }
   return true;
}

void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if(words.size() <= 1) 
   {   
      exit_status::set (0);
   }
   else if(is_number(words[1])) 
   {
      int a = atoi(words[1].c_str());
      exit_status::set (a);
   }
   else 
   {
      exit_status::set (127);
   }
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words)
{
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   stringstream stream; stream << "";

   
   if(words.size() <= 1) 
   {
      stream << state.get_cwd()->get_name() << ":" << "\n";
      state.get_cwd()->print_dir(stream);
      cout << stream.str() << "\n";
      return;
   }
   inode_ptr inode = nullptr;
   
     
   for(auto & func : wordvec(words.begin()+1, words.end())) {
       
         inode = state.return_parent_path(func);
         
         if(inode == nullptr)
         {
            cerr<<"Folder "<<words[1]<<" does not exist"<<endl;
            return;
         }
     
      if(inode->get_type() == file_type::DIRECTORY_TYPE) {
         stream << func << ":" << "\n";
         inode->print_dir(stream);
      }
      else {
         inode->print_desc(stream, func);
      }
   }

  cout << stream.str() << "\n";
   
}

void fn_lsr (inode_state& state, const wordvec& words)
{
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   deque<dirent_pair> stack;
   stringstream stream; 
   stream << "";
    
   // call cwd
   if(words.size() <= 1) {
      string temp = ".";
      state.get_cwd()->recursive_print(stream, stack, temp);
      cout << stream.str() << "\n";
      return;
   }

   string func;
   inode_ptr inode = nullptr;

   for(auto & func : wordvec(words.begin()+1, words.end())) 
   {
     
         inode = state.return_parent_path(func);
      
   }
      if(inode->get_type() == file_type::DIRECTORY_TYPE) 
      {
         inode->recursive_print(stream, stack, func);
         stack.clear();
      }
      else 
      {
         inode->print_desc(stream, func);
      }
      
      cout << stream.str() << "\n";
}



void fn_make (inode_state& state, const wordvec& words)
{
     DEBUGF ('c', state);
     DEBUGF ('c', words);

     string func = split(words[1], "/").back();
  
      inode_ptr parentNode = state.return_parent_path(words[1]);
      auto parentDir = directory_ptr_of(parentNode->get_contents());

      parentDir->mkfile(func);

    
      if(words.size() > 2)
      {
         auto new_node = parentDir->get_dirents(func);
         auto new_file = plain_file_ptr_of(new_node->get_contents());
         new_file->writefile(wordvec(words.begin()+2, words.end()));
      }
}


void fn_mkdir (inode_state& state, const wordvec& words)
{
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if(words.size() <=1)
   {
      cerr<<"mkdir: needs argument"<<endl;
   }

   wordvec path = split(words[1], "/");
   
   if(!path.size()){
      cerr << "mkdir: invalid directory" << endl;    
   }
     
   string dirName = path.at(path.size()-1);
   
   inode_ptr parentNode = state.return_parent_path(words[1]);
   auto parentDir = directory_ptr_of(parentNode->get_contents());
   parentDir->mkdir(dirName, parentNode);
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if(words.size() <= 1) 
   {
      state.set_prompt(" ");
      return;
   }

   stringstream stream; stream << "";

   for(auto & s : wordvec(words.begin()+1, words.end())) 
   {
      stream << s << " ";
   }

   state.set_prompt(stream.str());
   return;
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   //create a new stream
   stringstream stream;
   //Create a vector for all parsed_paths
   wordvec parsed_paths = {""};
   
   //Start at cwd
   inode_ptr iterator = state.get_cwd();
   //If cwd is root
   if(iterator->get_name() == state.get_root()->get_name())
   {
       cout<<"/" <<endl;
       return;
   }
    //Go through all directories until we hit cwd
    while(iterator->get_name() != state.get_root()->get_name()) 
    {
      parsed_paths.push_back(iterator->get_name());
      parsed_paths.push_back("/");

      auto contents = directory_ptr_of(iterator->get_contents());
      iterator = contents->get_dirents("..");
   }
   // root case
   if(!parsed_paths.size()) 
   {
      cout << "/" << endl;
      return;
   }
   //Reverse the vector
   reverse(parsed_paths.begin(), parsed_paths.end());

   //Parse through vector
   for(auto & part : parsed_paths) 
   {
      stream << part;
   }
   
   cout << stream.str() << endl;
}

void fn_rm (inode_state& state, const wordvec& words)
{
     DEBUGF ('c', state);
     DEBUGF ('c', words);

   string path = split(words[1], "/").back();
   inode_ptr parentNode = nullptr;
   inode_ptr inode = nullptr;
  
   inode = state.return_parent_path(words.at(1));
   parentNode = state.return_parent_path(words.at(1));
  
   auto dirPointer = directory_ptr_of(parentNode->get_contents());
   dirPointer->remove(inode->get_name());
   inode = nullptr;
}

void fn_rmr (inode_state& state, const wordvec& words)
{
     DEBUGF ('c', state);
     DEBUGF ('c', words);

   string path = split(words[1], "/").back();
   inode_ptr parentNode = nullptr, 
   inode = nullptr;
  
   parentNode = state.return_parent_path(words[1]);
   inode = state.return_parent_path(words[1]);
   
   state.recursive_free(inode);

   auto dirPointer = directory_ptr_of(parentNode->get_contents());
   dirPointer->remove(inode->get_name());
   inode = nullptr;

}

