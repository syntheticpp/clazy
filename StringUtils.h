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

#ifndef CLANG_LAZY_STRING_UTILS_H
#define CLANG_LAZY_STRING_UTILS_H

#include "checkmanager.h"
#include "Utils.h"

#include <clang/AST/ExprCXX.h>
#include <clang/AST/DeclCXX.h>
#include <string>

template <typename T>
inline std::string classNameFor(T *ctorDecl)
{
    return "";
}

template <>
inline std::string classNameFor(clang::CXXConstructorDecl *ctorDecl)
{
    return ctorDecl->getParent()->getNameAsString();
}

template <>
inline std::string classNameFor(clang::CXXMethodDecl *method)
{
    return method->getParent()->getNameAsString();
}

template <>
inline std::string classNameFor(clang::CXXConstructExpr *expr)
{
    return classNameFor(expr->getConstructor());
}

template <typename T>
inline bool isOfClass(T *node, const std::string &className)
{
    return node && classNameFor<T>(node) == className;
}

namespace StringUtils {

inline bool functionIsOneOf(clang::FunctionDecl *func, const std::vector<std::string> &functionNames)
{
    return func && std::find(functionNames.cbegin(), functionNames.cend(), func->getNameAsString()) != functionNames.cend();
}

inline void printLocation(const clang::SourceLocation &loc, bool newLine = true)
{
    llvm::errs() << loc.printToString(*(CheckManager::instance()->m_sm));
    if (newLine)
        llvm::errs() << "\n";
}

inline void printRange(const clang::SourceRange &range, bool newLine = true)
{
    printLocation(range.getBegin(), false);
    llvm::errs() << "-";
    printLocation(range.getEnd(), newLine);
}

inline void printLocation(const clang::Stmt *s, bool newLine = true)
{
    if (s)
        printLocation(s->getLocStart(), newLine);
}

inline void printLocation(clang::PresumedLoc loc, bool newLine = true)
{
    llvm::errs() << loc.getFilename() << " " << loc.getLine() << ":" << loc.getColumn();
    if (newLine)
        llvm::errs() << "\n";
}

inline std::string qualifiedMethodName(clang::CXXMethodDecl *method)
{
    // method->getQualifiedNameAsString() returns the name with template arguments, so do a little hack here
    if (!method || !method->getParent())
        return "";

    return method->getParent()->getNameAsString() + "::" + method->getNameAsString();
}

inline void printParents(clang::ParentMap *map, clang::Stmt *s)
{
    int level = 0;
    llvm::errs() << (s ? s->getStmtClassName() : nullptr) << "\n";

    while (clang::Stmt *parent = Utils::parent(map, s)) {
        ++level;
        llvm::errs() << std::string(level, ' ') << parent->getStmtClassName() << "\n";
        s = parent;
    }
}

}

#endif

