/* This source file is part of the Tomviz project, https://tomviz.org/.
   It is released under the 3-Clause BSD License, see "LICENSE". */

#ifndef tomvizVariant_h
#define tomvizVariant_h

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace tomviz {

class Variant
{

public:
  enum Type
  {
    INVALID,
    INTEGER,
    DOUBLE,
    BOOL,
    STRING,
    LIST,
    MAP
  };

  Variant();
  Variant(const std::string& str);
  Variant(const std::vector<Variant>& l);
  Variant(int i);
  Variant(double d);
  Variant(bool b);
  Variant(const Variant& v);
  Variant(const std::map<std::string, Variant>& m);
  ~Variant();

  Variant& operator=(const Variant& v);

  bool toBool() const;
  int toInteger() const;
  double toDouble() const;
  std::string toString() const;
  std::vector<Variant> toList() const;
  std::map<std::string, Variant> toMap() const;
  Type type() const;

private:
  Type m_type;
  union VariantUnion
  {
    int integerVal;
    double doubleVal;
    bool boolVal;
    std::string stringVal;
    std::vector<Variant> listVal;
    std::map<std::string, Variant> mapVal;

    VariantUnion() {}

    ~VariantUnion() {}
  };

  VariantUnion m_value;
  void copy(const Variant& v);
};
} // namespace tomviz

#endif
