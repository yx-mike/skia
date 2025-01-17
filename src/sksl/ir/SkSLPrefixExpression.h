/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PREFIXEXPRESSION
#define SKSL_PREFIXEXPRESSION

#include "include/sksl/SkSLOperator.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <memory>

namespace SkSL {

/**
 * An expression modified by a unary operator appearing before it, such as '!flag'.
 */
class PrefixExpression final : public Expression {
public:
    inline static constexpr Kind kExpressionKind = Kind::kPrefix;

    // Use PrefixExpression::Make to automatically simplify various prefix expression types.
    PrefixExpression(Position pos, Operator op, std::unique_ptr<Expression> operand)
        : INHERITED(pos, kExpressionKind, &operand->type())
        , fOperator(op)
        , fOperand(std::move(operand)) {}

    // Creates an SkSL prefix expression; uses the ErrorReporter to report errors.
    static std::unique_ptr<Expression> Convert(const Context& context, Position pos, Operator op,
                                               std::unique_ptr<Expression> base);

    // Creates an SkSL prefix expression; reports errors via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context, Position pos, Operator op,
                                            std::unique_ptr<Expression> base);

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
        if (property == Property::kSideEffects &&
            (this->getOperator().kind() == Operator::Kind::PLUSPLUS ||
             this->getOperator().kind() == Operator::Kind::MINUSMINUS)) {
            return true;
        }
        return this->operand()->hasProperty(property);
    }

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<PrefixExpression>(pos, this->getOperator(),
                                                  this->operand()->clone());
    }

    std::string description() const override {
        return this->getOperator().operatorName() + this->operand()->description();
    }

private:
    Operator fOperator;
    std::unique_ptr<Expression> fOperand;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
