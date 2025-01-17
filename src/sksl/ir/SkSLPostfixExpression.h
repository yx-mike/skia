/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_POSTFIXEXPRESSION
#define SKSL_POSTFIXEXPRESSION

#include "include/sksl/SkSLOperator.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * An expression modified by a unary operator appearing after it, such as 'i++'.
 */
class PostfixExpression final : public Expression {
public:
    inline static constexpr Kind kExpressionKind = Kind::kPostfix;

    PostfixExpression(Position pos, std::unique_ptr<Expression> operand, Operator op)
        : INHERITED(pos, kExpressionKind, &operand->type())
        , fOperand(std::move(operand))
        , fOperator(op) {}

    // Creates an SkSL postfix expression; uses the ErrorReporter to report errors.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               std::unique_ptr<Expression> base,
                                               Operator op);

    // Creates an SkSL postfix expression; reports errors via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            std::unique_ptr<Expression> base,
                                            Operator op);

    Operator getOperator() const {
        return fOperator;
    }

    std::unique_ptr<Expression>& operand() {
        return fOperand;
    }

    const std::unique_ptr<Expression>& operand() const {
        return fOperand;
    }

    bool hasProperty(Property property) const override {
        return (property == Property::kSideEffects) ||
               this->operand()->hasProperty(property);
    }

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<PostfixExpression>(pos, this->operand()->clone(),
                                                   this->getOperator());
    }

    std::string description() const override {
        return this->operand()->description() + this->getOperator().operatorName();
    }

private:
    std::unique_ptr<Expression> fOperand;
    Operator fOperator;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
