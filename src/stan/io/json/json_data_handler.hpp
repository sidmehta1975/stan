#ifndef __STAN__IO__JSON__JSON_DATA_HANDLER_HPP__
#define __STAN__IO__JSON__JSON_DATA_HANDLER_HPP__

#include <cctype>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <boost/throw_exception.hpp>
#include <boost/lexical_cast.hpp>
#include <stan/math/matrix.hpp>
#include <stan/io/var_context.hpp>
#include <stan/io/json/json_error.hpp>
#include <stan/io/json/json_parser.hpp>
#include <stan/io/json/json_handler.hpp>

namespace stan {

  namespace json {

    namespace {
      typedef 
      std::map<std::string, 
               std::pair<std::vector<double>,
                         std::vector<size_t> > > 
      vars_map_r;

      typedef 
      std::map<std::string, 
               std::pair<std::vector<int>, 
                         std::vector<size_t> > > 
      vars_map_i;

    }

    class json_data_handler : public stan::json::json_handler {
    private:
      vars_map_r& vars_r_;
      vars_map_i& vars_i_;
      std::string key_;
      std::vector<double> values_r_;
      std::vector<int> values_i_;
      std::vector<size_t> dims_;
      std::vector<size_t> dims_verify_;
      std::vector<bool> dims_unknown_;
      size_t dim_idx_;
      size_t dim_last_;
      bool is_int_;

      void reset() {
        key_.clear();
        values_r_.clear();
        values_i_.clear();
        dims_.clear();
        dims_verify_.clear();
        dims_unknown_.clear();
        dim_idx_ = 0;
        dim_last_ = 0;
        is_int_ = true;
      }

      bool is_init() {
        return (key_.size() == 0
                && values_r_.size() == 0
                && values_i_.size() == 0
                && dims_.size() == 0
                && dims_verify_.size() == 0
                && dims_unknown_.size() == 0
                && dim_idx_ == 0
                && dim_last_ == 0
                && is_int_);
      }


    public:
      json_data_handler(vars_map_r& vars_r, vars_map_i& vars_i) : 
        json_handler(), vars_r_(vars_r), vars_i_(vars_i), 
        key_(), values_r_(), values_i_(), 
        dims_(), dims_verify_(), dims_unknown_(), 
        dim_idx_(), dim_last_(), is_int_() {
      }

      void start_text() {
        // std::cout << "start_text" << std::endl;
        vars_i_.clear();
        vars_r_.clear();
        reset();
      }

      void end_text() {
        // std::cout << "end_text" << std::endl;
        reset(); 
      }

      void start_array() {
        // std::cout << "start_array" << std::endl;
        if (0 == key_.size()) {
          throw json_error("expecting JSON object, found array");
        }
        if (dim_idx_ > 0 && dim_last_ == dim_idx_) {
            std::stringstream errorMsg;
            errorMsg << "variable: " << key_ << ", error: non-scalar array value";
            throw json_error(errorMsg.str());
          }
        incr_dim_size();
        dim_idx_++;
        if (dims_.size() < dim_idx_) {
          dims_.push_back(0);
          dims_unknown_.push_back(true);
          dims_verify_.push_back(0);
        } else {
          dims_verify_[dim_idx_-1] = 0;
        }
      }

      void end_array() {
        // std::cout << "end_array" << std::endl;
        if (dims_[dim_idx_-1] == 0) {
          std::stringstream errorMsg;
          errorMsg << "variable: " << key_ << ", error: empty array not allowed";
          throw json_error(errorMsg.str());
        }
        if (dims_unknown_[dim_idx_-1] == true) {
          dims_unknown_[dim_idx_-1] = false;
        } else if (dims_verify_[dim_idx_-1] != dims_[dim_idx_-1]) {
          std::stringstream errorMsg;
          errorMsg << "variable: " << key_ << ", error: non-rectangular array";
          throw json_error(errorMsg.str());
        }
        if (0 == dim_last_ 
            && ((is_int_ && values_i_.size() > 0) || (values_r_.size() > 0))) 
          dim_last_ = dim_idx_;
        dim_idx_--;
      }

      void start_object() {
        // std::cout << "start_object" << std::endl;
        if (!is_init()) {
          std::stringstream errorMsg;
          errorMsg << "variable: " << key_ << ", error: nested objects not allowed";
          throw json_error(errorMsg.str());
        }
      }

      void end_object() {
        // std::cout << "end_object" << std::endl;
        save_current_key_value_pair();
        reset(); 
      }

      void null() {
        // std::cout << "null literal" << std::endl;
        std::stringstream errorMsg;
        errorMsg << "variable: " << key_ << ", error: null values not allowed";
        throw json_error(errorMsg.str());
      }

      void boolean(bool p) {
        // std::cout << "bool literal" << std::endl;
        std::stringstream errorMsg;
        errorMsg << "variable: " << key_ << ", error: boolean values not allowed";
        throw json_error(errorMsg.str());
      }

      void string(const std::string& s) {
        // std::cout << "string: " << s << std::endl;
        double tmp;
        if (0 == s.compare("-inf")) {
          tmp = -std::numeric_limits<double>::infinity();
        } else if (0 == s.compare("inf")) {
          tmp = std::numeric_limits<double>::infinity();
        } else {
          std::stringstream errorMsg;
          errorMsg << "variable: " << key_ << ", error: string values not allowed";
          throw json_error(errorMsg.str());
        }
        if (is_int_) {
          for (std::vector<int>::iterator it = values_i_.begin(); it != values_i_.end(); ++it)
            values_r_.push_back(*it);
        }
        is_int_ = false;
        values_r_.push_back(tmp);
        incr_dim_size();
      }

      void key(const std::string& key) {
        // std::cout << "key: " << key << std::endl;
        save_current_key_value_pair();
        reset();
        key_ = key;
      }

      void number_double(double x) { 
        // std::cout << "double value: " << x << std::endl;
        set_last_dim();
        if (is_int_) {
          for (std::vector<int>::iterator it = values_i_.begin(); 
               it != values_i_.end(); ++it)
            values_r_.push_back(*it);
        }
        is_int_ = false;
        values_r_.push_back(x);
        incr_dim_size();
      }

      void number_long(long n) { 
        // std::cout << "long value: " << n << std::endl;
        set_last_dim();
        if (is_int_) {
          values_i_.push_back(n);
        } else {
          // check number in range?
          values_r_.push_back(n);
        }
        incr_dim_size();
      }

      void number_unsigned_long(unsigned long n) { 
        // std::cout << "ul value: " << n << std::endl;
        set_last_dim();
        if (is_int_) {
          // check number in range?
          values_i_.push_back(n);
        } else {
          // check number in range?
          values_r_.push_back(n);
        }
        incr_dim_size();
      }

      void save_current_key_value_pair() {
        if (0 == key_.size()) return;  // json object is empty?

        // std::cout << "save key_: " << key_ << std::endl;
        // redefinition or variables not allowed
        if (vars_r_.find(key_) != vars_r_.end()
            || vars_i_.find(key_) != vars_i_.end()) {
            std::stringstream errorMsg;
            errorMsg << "attempt to redefine variable: " << key_;
            throw json_error(errorMsg.str());
        }
        if (is_int_) {
          std::pair<std::vector<int>, 
                    std::vector<size_t> > pair = make_pair(values_i_, dims_);
          vars_i_[key_] = pair;
        } else {
          vars_r_[key_]
            = std::pair<std::vector<double>, 
                        std::vector<size_t> >(values_r_, dims_);
        }
      }

      void set_last_dim() {
        if (dim_last_ > 0 && dim_idx_ < dim_last_) {
          std::stringstream errorMsg;
          errorMsg << "variable: " << key_ << ", error: non-rectangular array";
          throw json_error(errorMsg.str());
        }
        dim_last_ = dim_idx_;
      }

      void incr_dim_size() {
        if (dim_idx_ > 0) {
          if (dims_unknown_[dim_idx_-1])
            dims_[dim_idx_-1]++;
          else 
            dims_verify_[dim_idx_-1]++;
        }
      }

    };

  }

}

#endif
