#ifndef __ANY_HPP__
#define __ANY_HPP__

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace Easy {
    class Any
    {
    public:
        Any() noexcept
            : content(nullptr) {}

        Any(const Any& other)
            : content(other.content ? other.content->clone() : nullptr) {}

        Any(Any&& other) noexcept
            : content(other.content)
        {
            other.content = nullptr;
        }

        template <typename ValueType, typename T = typename std::decay<ValueType>::type, typename = typename std::enable_if<!std::is_same<T, Any>::value>::type>
        Any(ValueType&& value)
            : content(new Holder<T>(std::forward<ValueType>(value)))
        {
        }

        ~Any()
        {
            delete content;
        }

        Any& operator=(const Any& other)
        {
            if (this != &other) {
                Any(other).swap(*this);
            }
            return *this;
        }

        Any& operator=(Any&& other) noexcept
        {
            if (this != &other) {
                delete content;
                content       = other.content;
                other.content = nullptr;
            }
            return *this;
        }

        template <typename ValueType, typename T = typename std::decay<ValueType>::type, typename = typename std::enable_if<!std::is_same<T, Any>::value>::type>
        Any& operator=(ValueType&& value)
        {
            Any(std::forward<ValueType>(value)).swap(*this);
            return *this;
        }

        void swap(Any& other) noexcept
        {
            std::swap(content, other.content);
        }

        bool has_value() const noexcept
        {
            return content != nullptr;
        }

        void reset() noexcept
        {
            delete content;
            content = nullptr;
        }

        const std::type_info& type() const noexcept
        {
            return content ? content->type() : typeid(void);
        }

        template <typename ValueType>
        friend ValueType any_cast(const Any& operand);

        template <typename ValueType>
        friend ValueType any_cast(Any& operand);

        template <typename ValueType>
        friend ValueType any_cast(Any&& operand);

        template <typename ValueType>
        friend const ValueType* any_cast(const Any* operand) noexcept;

        template <typename ValueType>
        friend ValueType* any_cast(Any* operand) noexcept;

    private:
        struct Placeholder {
            virtual ~Placeholder() {}
            virtual const std::type_info& type() const noexcept = 0;
            virtual Placeholder*          clone() const         = 0;
        };

        template <typename ValueType>
        struct Holder : public Placeholder {
            Holder(ValueType&& value)
                : held(std::forward<ValueType>(value)) {}

            Holder(const ValueType& value)
                : held(value) {}

            const std::type_info& type() const noexcept override
            {
                return typeid(ValueType);
            }

            Placeholder* clone() const override
            {
                return new Holder(held);
            }

            ValueType held;
        };

        Placeholder* content;
    };

    template <typename ValueType>
    ValueType any_cast(const Any& operand)
    {
        if (operand.type() != typeid(ValueType)) {
            throw std::bad_cast();
        }
        return static_cast<Any::Holder<typename std::remove_cv<ValueType>::type>*>(operand.content)->held;
    }

    template <typename ValueType>
    ValueType any_cast(Any& operand)
    {
        if (operand.type() != typeid(ValueType)) {
            throw std::bad_cast();
        }
        return static_cast<Any::Holder<typename std::remove_cv<ValueType>::type>*>(operand.content)->held;
    }

    template <typename ValueType>
    ValueType any_cast(Any&& operand)
    {
        if (operand.type() != typeid(ValueType)) {
            throw std::bad_cast();
        }
        return std::move(static_cast<Any::Holder<typename std::remove_cv<ValueType>::type>*>(operand.content)->held);
    }

    template <typename ValueType>
    const ValueType* any_cast(const Any* operand) noexcept
    {
        if (operand && operand->type() == typeid(ValueType)) {
            return &static_cast<Any::Holder<ValueType>*>(operand->content)->held;
        }
        return nullptr;
    }

    template <typename ValueType>
    ValueType* any_cast(Any* operand) noexcept
    {
        if (operand && operand->type() == typeid(ValueType)) {
            return &static_cast<Any::Holder<ValueType>*>(operand->content)->held;
        }
        return nullptr;
    }
} // namespace Easy

#endif