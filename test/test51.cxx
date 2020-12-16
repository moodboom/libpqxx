#include <iostream>
#include <sstream>

#include <pqxx/largeobject>
#include <pqxx/transaction>
#include <pqxx/transactor>

#include "test_helpers.hxx"

using namespace pqxx;

// Test program for libpqxx's Large Objects interface.
namespace
{
std::string const Contents{"Large object test contents"};


void test_051()
{
  connection conn;

  largeobject obj{perform([&conn] {
    work tx{conn};
    largeobjectaccess A(tx);
    auto new_obj = largeobject(A);

    A.write(Contents);

    char Buf[200];
    constexpr auto Size = std::size(Buf) - 1;

    auto Offset = A.seek(0, std::ios::beg);
    PQXX_CHECK_EQUAL(Offset, 0, "Wrong position after seek to beginning.");

    PQXX_CHECK_EQUAL(
      std::size_t(A.read(Buf, Size)), std::size(Contents),
      "Unexpected read() result.");

    PQXX_CHECK_EQUAL(
      std::string(Buf, std::size(Contents)), Contents,
      "Large object contents were mutilated.");

    // Now write contents again, this time as a conn string
    // TODO: Use C++20's ssize().
    PQXX_CHECK_EQUAL(
      A.seek(-int(std::size(Contents)), std::ios::end), 0,
      "Bad position after seeking to beginning of large object.");

    A.write(Buf, std::size(Contents));
    A.seek(0, std::ios::beg);
    PQXX_CHECK_EQUAL(
      std::size_t(A.read(Buf, Size)), std::size(Contents),
      "Bad length for rewritten large object.");

    PQXX_CHECK_EQUAL(
      std::string(Buf, std::size(Contents)), Contents,
      "Rewritten large object was mangled.");

    tx.commit();
    return new_obj;
  })};

  PQXX_CHECK(
    obj != largeobject{}, "Large objects: false negative on operator!=().");

  PQXX_CHECK(
    not(obj == largeobject{}),
    "Large objects: false positive on operator==().");

  PQXX_CHECK(
    not(obj != obj), "Large objects: false positive on operator!=().");
  PQXX_CHECK(obj == obj, "Large objects: false negative on operator==().");

  PQXX_CHECK(obj <= obj, "Large objects: false negative on operator<=().");
  PQXX_CHECK(obj >= obj, "Large objects: false negative on operator>=().");

  PQXX_CHECK(not(obj < obj), "Large objects: false positive on operator<().");
  PQXX_CHECK(not(obj > obj), "Large objects: false positive on operator>().");

  perform([&conn, &obj] {
    work tx{conn};
    obj.remove(tx);
    tx.commit();
  });
}


PQXX_REGISTER_TEST(test_051);
} // namespace
