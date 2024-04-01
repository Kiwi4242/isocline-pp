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
public:
    IsoclinePP();

    bool StyleDef(const std::string &name, const std::string &style);
    bool Printf(const std::string &fmt, const std::string st="");
    bool SetHistory(const std::string &file, const int n=-1);
    void EnableAutoTab(const bool);
    void EnableHint(const bool);

    virtual void AddHistory(const std::string &statement);
    virtual void AddHistory(const std::vector<std::string> &his);

    // Complete the string in inp, return match in completions and the prefix that was matched in pref, called when the user presses tab
    virtual bool Completer(const std::string &inp, std::vector<CompletionItem> &completions) = 0;

    // Provide a single hint, called after a new character is entered
    virtual bool Hint(const std::string &inp, CompletionItem &);

    void EnableMultiLine(const bool enable);
#ifdef TODO
    /*! \brief Completions callback type definition.
     *
     * \e contextLen is counted in Unicode code points (not in bytes!).
     *
     * For user input:
     * if ( obj.me
     *
     * input == "if ( obj.me"
     * contextLen == 2 (depending on \e set_word_break_characters())
     *
     * Client application is free to update \e contextLen to be 6 (or any other non-negative
     * number not greater than the number of code points in input) if it makes better sense
     * for given client application semantics.
     *
     * \param input - UTF-8 encoded input entered by the user until current cursor position.
     * \param[in,out] contextLen - length of the additional context to provide while displaying completions.
     * \return A list of user completions.
     */
    typedef std::function<completions_t ( std::string const& input, int& contextLen )> completion_callback_t;

    /*! \brief Hints callback type definition.
     *
     * \e contextLen is counted in Unicode code points (not in bytes!).
     *
     * For user input:
     * if ( obj.me
     *
     * input == "if ( obj.me"
     * contextLen == 2 (depending on \e set_word_break_characters())
     *
     * Client application is free to update \e contextLen to be 6 (or any other non-negative
     * number not greater than the number of code points in input) if it makes better sense
     * for given client application semantics.
     *
     * \param input - UTF-8 encoded input entered by the user until current cursor position.
     * \param contextLen[in,out] - length of the additional context to provide while displaying hints.
     * \param color - a color used for displaying hints.
     * \return A list of possible hints.
     */
    typedef std::function<hints_t ( std::string const& input, int& contextLen, Color& color )> hint_callback_t;
#endif
    
    std::string ReadLine(const std::string &prompt);

    /*! \brief Print formatted string to standard output.
     *
     * This function ensures proper handling of ANSI escape sequences
     * contained in printed data, which is especially useful on Windows
     * since Unixes handle them correctly out of the box.
     *
     * \param fmt - printf style format.
     */
    // void print( char const* fmt, ... );

    void history_add( std::string const& line );

    /*! \brief Synchronize REPL's history with given file.
     *
     * Synchronizing means loading existing history from given file,
     * merging it with current history sorted by timestamps,
     * saving merged version to given file,
     * keeping merged version as current REPL's history.
     *
     * This call is an equivalent of calling:
     * history_save( "some-file" );
     * history_load( "some-file" );
     *
     * \param filename - a path to the file with which REPL's current history should be synchronized.
     * \return True iff history file was successfully created.
     */
    bool history_sync( std::string const& filename );

    /*! \brief Save REPL's history into given file.
     *
     * Saving means loading existing history from given file,
     * merging it with current history sorted by timestamps,
     * saving merged version to given file,
     * keeping original (NOT merged) version as current REPL's history.
     *
     * \param filename - a path to the file where REPL's history should be saved.
     * \return True iff history file was successfully created.
     */
    bool history_save( std::string const& filename );

};
