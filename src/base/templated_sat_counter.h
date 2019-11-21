
#ifndef __BASE_TEMPLATED_SAT_COUNTER_HH__
#define __BASE_TEMPLATED_SAT_COUNTER_HH__

#include <math.h>

#include <cstdint>

#include "base/logging.hh"
#include "base/types.hh"

/**
 * Implements an n bit saturating counter and provides methods to
 * increment, decrement, and read it.
 */
template <class T>
class SatCounter
{
  public:
    /** The default constructor should never be used. */
    SatCounter() = delete;

    /**
     * Constructor for the counter. The explicit keyword is used to make
     * sure the user does not assign a number to the counter thinking it
     * will be used as a counter value when it is in fact used as the number
     * of bits.
     *
     * @param bits How many bits the counter will have.
     * @param initial_val Starting value for the counter.
     */
    explicit SatCounter(unsigned bits, T initial_val = 0)
        : initialVal(initial_val), maxVal((1 << bits) - 1),
          counter(initial_val), nbits(bits)
    {
        fatal_if(bits > 8*sizeof(T),
                 "Number of bits exceeds counter size");
        fatal_if(initial_val > maxVal,
                 "Saturating counter's Initial value exceeds max value.");
    }

    /** Copy constructor. */
    SatCounter(const SatCounter& other)
        : initialVal(other.initialVal), maxVal(other.maxVal),
          counter(other.counter)
    {
    }

    /** Copy assignment. */
    SatCounter& operator=(const SatCounter& other) {
        if (this != &other) {
            SatCounter temp(other);
            this->swap(temp);
        }
        return *this;
    }

    /** Move constructor. */
    SatCounter(SatCounter&& other)
    {
        initialVal = other.initialVal;
        maxVal = other.maxVal;
        counter = other.counter;
        SatCounter temp(0);
        other.swap(temp);
    }

    /** Move assignment. */
    SatCounter& operator=(SatCounter&& other) {
        if (this != &other) {
            initialVal = other.initialVal;
            maxVal = other.maxVal;
            counter = other.counter;
            SatCounter temp(0);
            other.swap(temp);
        }
        return *this;
    }

    /**
     * Swap the contents of every member of the class. Used for the default
     * copy-assignment created by the compiler.
     *
     * @param other The other object to swap contents with.
     */
    void
    swap(SatCounter& other)
    {
        std::swap(initialVal, other.initialVal);
        std::swap(maxVal, other.maxVal);
        std::swap(counter, other.counter);
    }

    /** Pre-increment operator. */
    SatCounter&
    operator++()
    {
        if (counter < maxVal) {
            ++counter;
        }
        return *this;
    }

    /** Post-increment operator. */
    SatCounter
    operator++(int)
    {
        SatCounter old_counter = *this;
        ++*this;
        return old_counter;
    }

    /** Pre-decrement operator. */
    SatCounter&
    operator--()
    {
        if (counter > 0) {
            --counter;
        }
        return *this;
    }

    /** Post-decrement operator. */
    SatCounter
    operator--(int)
    {
        SatCounter old_counter = *this;
        --*this;
        return old_counter;
    }

    /** Shift-right-assignment. */
    SatCounter&
    operator>>=(const int& shift)
    {
        this->counter >>= shift;
        return *this;
    }

    /** Shift-left-assignment. */
    SatCounter&
    operator<<=(const int& shift)
    {
        this->counter <<= shift;
        if (this->counter > maxVal) {
            this->counter = maxVal;
        }
        return *this;
    }

    /** Add-assignment. */
    SatCounter&
    operator+=(const int& value)
    {
        if (maxVal - this->counter >= value) {
            this->counter += value;
        } else {
            this->counter = maxVal;
        }
        return *this;
    }

    /** Subtract-assignment. */
    SatCounter&
    operator-=(const int& value)
    {
        if (this->counter > value) {
            this->counter -= value;
        } else {
            this->counter = 0;
        }
        return *this;
    }

    /**
     * Read the counter's value.
     */
    operator T() const { return counter; }
    T getCounter() const {return counter;}
    uint8_t getMSB() const {
        return (counter & (1 << (nbits - 1))) >> (nbits - 1);
    }

    /** Reset the counter to its initial value. */
    void reset() { counter = initialVal; }

    /**
     * Calculate saturation percentile of the current counter's value
     * with regard to its maximum possible value.
     *
     * @return A value between 0.0 and 1.0 to indicate which percentile of
     *         the maximum value the current value is.
     */
    double calcSaturation() const { return (double) counter / maxVal; }

    /**
     * Whether the counter has achieved its maximum value or not.
     *
     * @return True if the counter saturated.
     */
    bool isSaturated() const { return counter == maxVal; }

    /**
     * Saturate the counter.
     *
     * @return The value added to the counter to reach saturation.
     */
    T saturate()
    {
        const T diff = maxVal - counter;
        counter = maxVal;
        return diff;
    }

  private:
    T initialVal;
    T maxVal;
    T counter;
    unsigned nbits;
};

#endif // __BASE_TEMPLATED_SAT_COUNTER_HH__
