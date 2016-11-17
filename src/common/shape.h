#pragma once

// This file is part of the Marian toolkit.
// Marian is copyright (c) 2016 Marcin Junczys-Dowmunt.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <cstdint>

#include "exception.h"

  /**
   * @brief Represents the size of each dimension in a tensor.
   *
   * Note: this class currently is hard-coded to 3 dimensions.
   */

  const size_t SHAPE_SIZE = 3;


  struct Shape {
    size_t shape_[SHAPE_SIZE];

      /**
       * @brief Constructs a default shape.
       *
       * This default shape has four dimensions.
       * The size of each dimension is 1.
       */
      Shape() : shape_{0, 0, 0} { }

      /**
       * @brief Constructs a shape.
       *
       * @param i A list of integers representing the size of each dimension.
       */
      Shape(const std::initializer_list<size_t> &il) {
       std::copy(il.begin(), il.end(), begin());
      }

      Shape(const Shape& shape) {
       std::copy(shape.begin(), shape.end(), begin());
      }

      /**
       * @brief Gets a reference to the int representing the size of the <code>i</code>th dimension represented by this object.
       *
       * @return a reference to the int representing the size of the <code>i</code>th dimension represented by this object
       */
      //__host__ __device__
      size_t& operator[](size_t i) {
        return shape_[i];
      }

      /**
       * @brief Gets the size of the <code>i</code>th dimension represented by this object.
       *
       * @return the size of the <code>i</code>th dimension represented by this object
       */
      //__host__ __device__
      const size_t& operator[](size_t i) const {
        return shape_[i];
      }

	  /**
	   * @brief Gets the number of dimensions represented by this object
	   *
	   * @return the number of dimensions represented by this object
	   */
      size_t numDimensions() const {
        return SHAPE_SIZE;
      }

      /**
       * @brief Gets the total number of elements in a tensor of this shape.
       *
       * For example, if this shape represents a 5x100 tensor, this method would return 500.
       *
       * @return the total number of elements in a tensor of this shape
       */
      size_t elements() const {
        size_t s = 1;
        for(size_t i = 0; i < numDimensions(); ++i)
          s *= shape_[i];
        return s;
      }

      /** @brief Gets a pointer to an int that specifies the size of the first dimension represented by this object */
      size_t* begin() { return shape_; }

      /** @brief Gets a pointer to an int that specifies the size of the last dimension represented by this object */
      size_t* end() { return shape_ + SHAPE_SIZE; }

      /** @brief Gets a const pointer to an int that specifies the size of the first dimension represented by this object */
      const size_t* begin() const { return shape_; }

      /** @brief Gets a const pointer to an int that specifies the size of the last dimension represented by this object */
      const size_t* end() const { return shape_+ SHAPE_SIZE; }

      /**
       * @brief Tests this object for equality against another <code>Shape</code> object.
       *
       * @return <code>true</code> if the size of each dimension in this object
       *         is equal to the size of the corresponding dimension in the other object,
       *         <code>false</code> otherwise
       */
      bool operator==(const Shape& other) const {
        return std::equal(begin(), end(), other.begin());
      }

      /**
       * @brief Tests this object for inequality against another <code>Shape</code> object.
       */
      bool operator!=(const Shape& other) const {
        return !(*this == other);
      }

      size_t matrixSize() const {
        return shape_[0] * shape_[1];
      }

      size_t numSlices() const {
        size_t s = 1;
        for(size_t i = 2; i < numDimensions(); ++i)
          s *= shape_[i];
        return s;
      }

      void swap(Shape &other) {
        for(size_t i = 0; i < numDimensions(); ++i) {
          size_t temp = shape_[i];
          shape_[i] = other.shape_[i];
          other.shape_[i] = temp;
        }
      }
};

