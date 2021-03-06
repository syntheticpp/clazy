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

#include "inefficientqlist.h"
#include "Utils.h"
#include "checkmanager.h"

#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/AST.h>
#include <clang/AST/DeclTemplate.h>

using namespace clang;
using namespace std;

enum IgnoreMode {
    None = 0,
    NonLocalVariable = 1,
    InFunctionWithSameReturnType = 2,
    IsAssignedTooInFunction = 4
};

InefficientQList::InefficientQList(const std::string &name)
    : CheckBase(name)
{
}

static bool shouldIgnoreVariable(VarDecl *varDecl, int ignoreMode)
{
    DeclContext *context = varDecl->getDeclContext();
    FunctionDecl *fDecl = context ? dyn_cast<FunctionDecl>(context) : nullptr;

    if (ignoreMode & NonLocalVariable) {
        if (fDecl == nullptr)
            return true;
    }

    if (ignoreMode & IsAssignedTooInFunction) {
        if (fDecl != nullptr && fDecl->getReturnType() == varDecl->getType())
            return true;
    }

    if (ignoreMode & IsAssignedTooInFunction) {
        if (fDecl != nullptr) {
            if (Utils::containsAssignment(fDecl->getBody(), varDecl))
                return true;
        }
    }

    return false;
}

void InefficientQList::VisitDecl(clang::Decl *decl)
{
    VarDecl *varDecl = dyn_cast<VarDecl>(decl);
    if (varDecl == nullptr)
        return;

    QualType type = varDecl->getType();
    const Type *t = type.getTypePtrOrNull();
    if (t == nullptr)
        return;

    if (shouldIgnoreVariable(varDecl, NonLocalVariable | InFunctionWithSameReturnType | IsAssignedTooInFunction))
        return;

    CXXRecordDecl *recordDecl = t->getAsCXXRecordDecl();
    if (recordDecl == nullptr || recordDecl->getNameAsString() != "QList")
        return;

    ClassTemplateSpecializationDecl *tstdecl = dyn_cast<ClassTemplateSpecializationDecl>(recordDecl);
    if (tstdecl == nullptr)
        return;

    const TemplateArgumentList &tal = tstdecl->getTemplateArgs();

    if (tal.size() != 1) return;
    QualType qt2 = tal[0].getAsType();
    const int size_of_void = 8 * 8; // 64 bits
    const int size_of_T = m_ci.getASTContext().getTypeSize(qt2);

    if (size_of_T > size_of_void) {
        string s = string("Use QVector instead of QList for type with size " + to_string(size_of_T / 8) + " bytes");
        emitWarning(decl->getLocStart(), s.c_str());
    }
}

REGISTER_CHECK_WITH_FLAGS("inefficient-qlist", InefficientQList, HiddenFlag)
