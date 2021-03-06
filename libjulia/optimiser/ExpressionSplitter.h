/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * Optimiser component that turns complex expressions into multiple variable
 * declarations.
 */
#pragma once

#include <libjulia/ASTDataForward.h>

#include <libjulia/optimiser/ASTWalker.h>
#include <libjulia/optimiser/NameDispenser.h>

#include <vector>

namespace dev
{
namespace julia
{

class NameCollector;


/**
 * Optimiser component that modifies an AST in place, turning complex
 * expressions into simple expressions and multiple variable declarations.
 *
 * Code of the form
 *
 * sstore(mul(x, 4), mload(y))
 *
 * is transformed into
 *
 * let a1 := mload(y)
 * let a2 := mul(x, 4)
 * sstore(a2, a1)
 *
 * The transformation is not applied to loop conditions, because the loop control flow
 * does not allow "outlining" the inner expressions in all cases.
 *
 * The final program should be in a form such that with the exception of a loop condition,
 * function calls can only appear in the right-hand side of a variable declaration,
 * assignments or expression statements and all arguments have to be constants or variables.
 */
class ExpressionSplitter: public ASTModifier
{
public:
	explicit ExpressionSplitter(NameDispenser& _nameDispenser):
		m_nameDispenser(_nameDispenser)
	{ }

	virtual void operator()(FunctionalInstruction&) override;
	virtual void operator()(FunctionCall&) override;
	virtual void operator()(If&) override;
	virtual void operator()(Switch&) override;
	virtual void operator()(ForLoop&) override;
	virtual void operator()(Block& _block) override;

private:
	/// Replaces the expression by a variable if it is a function call or functional
	/// instruction. The declaration of the variable is appended to m_statementsToPrefix.
	/// Recurses via visit().
	void outlineExpression(Expression& _expr);

	/// List of statements that should go in front of the currently visited AST element,
	/// at the statement level.
	std::vector<Statement> m_statementsToPrefix;
	NameDispenser& m_nameDispenser;
};

}
}
