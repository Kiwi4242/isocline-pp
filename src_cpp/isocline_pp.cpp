#include <cstring>

#include <isocline_pp.h>
#include <isocline.h>
#include "../src/history.h"
#include "../src/env.h"

void InputCompleter(ic_completion_env_t* cenv, const char* input ) 
{
    IsoclinePP *iso = static_cast<IsoclinePP*>(ic_completion_arg(cenv));
    long cursor;
    const char *allInp = ic_completion_input(cenv, &cursor);
    if (ic_completion_is_hint(cenv)) {
        CompletionItem hint;
        bool atEnd = cursor >= strlen(allInp)-1;
        if (iso->Hint(input, hint, atEnd)) {
            unsigned int prefLen = hint.delBefore;
            std::string st = hint.comp;
            ic_add_completion_prim(cenv, st.c_str(), st.c_str(), st.c_str(), prefLen, 0);
        }
        return;
    }

    std::vector<CompletionItem> comps;
    if (!iso->Completer(input, comps)) {
        return;
    }

    for (unsigned int i = 0; i < comps.size(); i++) {
        std::string &f = comps[i].comp;
        // need to pass f1 as the new part of the completion
        // std::string f1 = f.substr(prefLen);
        bool res = ic_add_completion_prim(cenv, f.c_str(), f.c_str(), f.c_str(), comps[i].delBefore, 0);
        // bool res = ic_add_completion_ex(cenv, f1.c_str(), f.c_str(), f.c_str());
        if (!res) {
            break;
        }
    }    
}


IsoclinePP::IsoclinePP()
{
  ic_set_default_completer(&InputCompleter, this);
}

bool IsoclinePP::StyleDef(const std::string &name, const std::string &style)
{
    ic_style_def(name.c_str(), style.c_str());
    return true;

}

bool IsoclinePP::Printf(const std::string &fmt, const std::string st)
{
    ic_printf(fmt.c_str(), st.c_str());
    return true;

}


void IsoclinePP::EnableAutoTab(const bool b)
{
    ic_enable_auto_tab(b);
}

void IsoclinePP::EnableHint(const bool en)
{
    ic_enable_hint(en);
    ic_set_hint_delay(1000);
}


void IsoclinePP::Print(const std::string &msg)
{
    ic_print(msg.c_str());
}


std::string IsoclinePP::ReadLine(const std::string &prompt)
{
    char *inp = ic_readline(prompt.c_str());
    if (inp) {
        curInp = inp;
        free(inp);
    } else {
        curInp = "";
    }
    return curInp;
}

bool IsoclinePP::Hint(const std::string &, CompletionItem &item, const bool atEnd)
{
    item.Set("", 0, 0);
    return false;
}

void IsoclinePP::EnableMultiLine(const bool enable)
{
    ic_enable_multiline(enable);
}


bool IsoclinePP::SetHistoryFile(const std::string &file, const int n)
{
    ic_set_history(file.c_str(), n);
    return true;
}

void IsoclinePP::AddHistory(const std::vector<std::string> &his)
{
    typedef std::vector<std::string>::const_iterator vecIt;
    for (vecIt it = his.begin(); it != his.end(); it++) {
        ic_history_add(it->c_str());
    }
}

void IsoclinePP::AddHistory(const std::string &statement)
{
    ic_history_add(statement.c_str());
}

// functions to use a custom history

ssize_t HistoryCountCallback(void *data)
{
    IsoclinePP *iso = static_cast<IsoclinePP*>(data);
    if (iso) {
        return iso->HistoryCount();
    }
    return 0;
}

std::string HistoryGetCallback(void *data, const ssize_t n)
{
    IsoclinePP *iso = static_cast<IsoclinePP*>(data);
    if (iso) {
        return iso->GetHistoryItem(n);
    }
    return "";
}

void HistoryDelCallback(void *data, const ssize_t n1, const ssize_t n2)
{

}

void HistoryPushCallback(void *data, const std::string &s)
{

}

void IsoclinePP::UseCustomHistory()
{
    history_t *his = ic_get_env()->history;
    history_set_custom(his, HistoryCountCallback, HistoryGetCallback, HistoryDelCallback, HistoryPushCallback, this);
}

int IsoclinePP::HistoryCount() 
{
    return 0;
}

std::string IsoclinePP::GetHistoryItem(const ssize_t n)
{
    return "";
}

void IsoclinePP::HistoryDelete(const ssize_t ind, const ssize_t n)
{}

void IsoclinePP::HistoryAdd(const std::string &st)
{}

