#include <isocline_pp.h>

class MyIsocline : public IsoclinePP {
public:
    MyIsocline() : IsoclinePP() {
        EnableHint(true);
    }
    bool Completer(const std::string &inp, std::vector<CompletionItem> &completions);

    // Provide a single hint, called after a new character is entered
    bool Hint(const std::string &inp, CompletionItem &hint, const bool atEnd);
};

bool StartsWith(const std::string &mainStr, const std::string &start, const bool ignoreCase)
{
  std::string main = mainStr;

  // if (ignoreCase) {
  //   // Convert toMatch to lower case
  //   main = ToLower(mainStr);
  // } else {
  //   main = mainStr;
  // }

  if(main.find(start) == 0)
      return true;
  else
      return false;
}


bool MyIsocline::Hint(const std::string &inp, CompletionItem &hint, const bool atEnd)
{
    hint.Set("", 0, 0);
    if (!atEnd) {
        return false;
    }
    
    if (inp == "To" || inp == "Tod" || inp == "Toda" || inp == "Today") {
        hint.Set("Today", inp.size(), 0);
        return true;
    }
    return false;
}

bool MyIsocline::Completer(const std::string &inp, std::vector<CompletionItem> &completions)
{
    if (inp == "S") {
        completions.push_back(CompletionItem("Slang", 1, 0));;
        completions.push_back(CompletionItem("Support", 1, 0));
        return true;
    }
    return false;
}

// main example
int main() 
{

  MyIsocline iso;

  // setlocale(LC_ALL,"C.UTF-8");  // we use utf-8 in this example

  // use `ic_print` functions to use bbcode's for markup
  iso.StyleDef("kbd","gray underline");     // you can define your own styles
  
  iso.Printf("[b]Welcome to CmdNG[/b]\n\n");
  
  // enable history; use a NULL filename to not persist history to disk
  iso.SetHistoryFile("history.txt", -1 /* default entries (= 200) */);

  // enable completion with a default completion function
  // set_default_completer(&InputCompleter, &shell);

  // enable syntax highlighting with a highlight function
  // ic_set_default_highlighter(highlighter, NULL);

  // try to auto complete after a completion as long as the completion is unique
  iso.EnableAutoTab(true);

  // inline hinting is enabled by default
  iso.EnableHint(true);

  // run until empty input
  std::string input;

  while(true) {
    std::string input = iso.ReadLine("Prompt");   // ctrl-d returns NULL (as well as errors)
    if (input == "exit") {
        break;
    } 
    iso.Printf("%s\n", input );    
    // shell.ProcessCommand(input);
  }
  iso.Printf("done");
  return 0;
}
