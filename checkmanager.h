/*
   This file is part of the clang-lazy static checker.

  Copyright (C) 2015 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Sérgio Martins <sergio.martins@kdab.com>

  Copyright (C) 2015 Sergio Martins <smartins@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef CLANG_LAZY_CHECK_MANAGER_H
#define CLANG_LAZY_CHECK_MANAGER_H

#include "checkbase.h"

#include <string>
#include <functional>
#include <vector>
#include <unordered_map>

struct RegisteredCheck;

struct RegisteredFixIt {
    typedef std::vector<RegisteredFixIt> List;
    RegisteredFixIt() : id(-1) {}
    RegisteredFixIt(int id, const std::string &name) : id(id), name(name) {}
    int id = -1;
    std::string name;
    bool operator==(const RegisteredFixIt &other) const { return id == other.id; }
};

using FactoryFunction = std::function<CheckBase*()>;

enum CheckFlag {
    NoFlag = 0,
    HiddenFlag = 1 // Check won't be printed in help, or in "all" mode, but can be used if explicitely specified
};

class CheckManager
{
public:
    static CheckManager *instance();
    int registerCheck(const std::string &name, int checkFlags, FactoryFunction);
    int registerFixIt(int id, const std::string &fititName, const std::string &checkName);

    void setCompilerInstance(clang::CompilerInstance *);
    std::vector<std::string> availableCheckNames(bool includeHidden) const;
    std::vector<std::string> requestedCheckNamesThroughEnv() const;

    RegisteredFixIt::List availableFixIts(const std::string &checkName) const;
    void createChecks(std::vector<std::string> requestedChecks);
    const CheckBase::List &createdChecks() const;
    bool fixitsEnabled() const;

    // Public for convinence
    clang::CompilerInstance *m_ci;
    clang::SourceManager *m_sm;

    bool allFixitsEnabled() const;

    std::vector<std::string> checkNamesForCommaSeparatedString(const std::string &str) const;

private:
    CheckManager();
    std::unique_ptr<CheckBase> createCheck(const std::string &name);
    std::string checkNameForFixIt(const std::string &) const;
    std::vector<RegisteredCheck> m_registeredChecks;
    CheckBase::List m_createdChecks;
    std::unordered_map<std::string, std::vector<RegisteredFixIt> > m_fixitsByCheckName;
    std::unordered_map<std::string, RegisteredFixIt > m_fixitByName;
    std::string m_requestedFixitName;
    bool m_enableAllFixits;
};

#define REGISTER_CHECK(CHECK_NAME, CLASS_NAME) \
    REGISTER_CHECK_WITH_FLAGS(CHECK_NAME, CLASS_NAME, NoFlag)

#define REGISTER_CHECK_WITH_FLAGS(CHECK_NAME, CLASS_NAME, FLAGS) \
    static int dummy = CheckManager::instance()->registerCheck(CHECK_NAME, (int)FLAGS, [](){ return new CLASS_NAME(CHECK_NAME); }); \
    inline void silence_warning() { (void)dummy; }

#define REGISTER_FIXIT(FIXIT_ID, FIXIT_NAME, CHECK_NAME) \
    static int dummy_##FIXIT_ID = CheckManager::instance()->registerFixIt(FIXIT_ID, FIXIT_NAME, CHECK_NAME); \
    inline void silence_warning_dummy_##FIXIT_ID() { (void)dummy_##FIXIT_ID; }

#endif
