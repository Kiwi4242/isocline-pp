/* ----------------------------------------------------------------------------
  Copyright (c) 2024, John Burnell
  This is free software; you can redistribute it and/or modify it
  under the terms of the MIT License. A copy of the license can be
  found in the "LICENSE" file at the root of this distribution.
-----------------------------------------------------------------------------*/

#pragma once

#include <string>
#include <functional>
#include <vector>

// #include "isocline.h"


class CompletionItem {
    // Class for storing completion information
public:
    int delBefore;
    int delAfter;
    std::string comp;

    CompletionItem() {}
    CompletionItem(const std::string &c, const int db, const int da) {
        Set(c, db, da);
    }
    void Set(const std::string &c, const int db, const int da) {
        delBefore = db;
        delAfter = da;
        comp = c;
    }
};


class IsoclinePP {
protected:
    std::string curInp;
    // bool useCustomHistory;
public:
    IsoclinePP();

    bool StyleDef(const std::string &name, const std::string &style);
    bool Printf(const std::string &fmt, const std::string st="");
    void EnableAutoTab(const bool);
    void EnableHint(const bool);

    // use the built-in history
    bool SetHistoryFile(const std::string &file, const int n=-1);
    virtual void AddHistory(const std::string &statement);
    virtual void AddHistory(const std::vector<std::string> &his);

    // functions to use a custom history
    void UseCustomHistory();
    virtual int HistoryCount();
    virtual std::string GetHistoryItem(const ssize_t n);
    virtual void HistoryDelete(const ssize_t ind, const ssize_t n);
    virtual void HistoryAdd(const std::string &st);

    // Complete the string in inp, return match in completions and the prefix that was matched in pref, called when the user presses tab
    virtual bool Completer(const std::string &inp, std::vector<CompletionItem> &completions) = 0;

    // Provide a single hint, called after a new character is entered
    virtual bool Hint(const std::string &inp, CompletionItem &, const bool atEnd);

    void EnableMultiLine(const bool enable);
    
    std::string ReadLine(const std::string &prompt);

    /*! \brief Print formatted string to standard output
     *
     * Print to the terminal while respection bbcode markup. 
     * This function ensures proper handling of ANSI escape sequences
     * contained in printed data, which is especially useful on Windows
     * since Unixes handle them correctly out of the box.
     *
     * For example:
     * ```
     * Print("[b]bold, [i]bold and italic[/i], [red]red and bold[/][/b] default.");
     * 
     * \param msg - string to be printed
     */
    void Print(const std::string &msg);

};
