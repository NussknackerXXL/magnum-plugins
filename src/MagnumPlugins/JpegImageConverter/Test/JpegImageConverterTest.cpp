/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include <sstream>
#include <Corrade/TestSuite/Tester.h>
#include <Corrade/TestSuite/Compare/Container.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/Trade/ImageData.h>

#include "MagnumPlugins/JpegImageConverter/JpegImageConverter.h"
#include "MagnumPlugins/JpegImporter/JpegImporter.h"

namespace Magnum { namespace Trade { namespace Test {

struct JpegImageConverterTest: TestSuite::Tester {
    explicit JpegImageConverterTest();

    void wrongFormat();
    void wrongType();

    void data();
    void data16();
};

namespace {
    constexpr const char OriginalData[] = {
        /* Skip */
        0, 0, 0, 0, 0, 0, 0, 0,

        '\x1f', '\x3f', '\x5f', '\x3f', '\x5f', '\x7f', 0, 0,
        '\x5f', '\x7f', '\x9f', '\x7f', '\x9f', '\xbf', 0, 0,
        '\x9f', '\xbf', '\xdf', '\xbf', '\xdf', '\xff', 0, 0
    };

    const ImageView2D original{PixelStorage{}.setSkip({0, 1, 0}),
        PixelFormat::RGB, PixelType::UnsignedByte, {2, 3}, OriginalData};

    constexpr const char ConvertedData[] = {
        '\x1f', '\x3f', '\x5f', '\x3f', '\x5f', '\x7f', 0, 0,
        '\x5f', '\x7f', '\x9f', '\x7f', '\x9f', '\xbf', 0, 0,
        '\x9f', '\xbf', '\xdf', '\xbf', '\xdf', '\xff', 0, 0
    };
}

JpegImageConverterTest::JpegImageConverterTest() {
    addTests({&JpegImageConverterTest::wrongFormat,
              &JpegImageConverterTest::wrongType,

              &JpegImageConverterTest::data});
}

void JpegImageConverterTest::wrongFormat() {
    ImageView2D image{PixelFormat::DepthComponent, PixelType::UnsignedByte, {}, nullptr};

    std::ostringstream out;
    Error redirectError{&out};

    const auto data = JpegImageConverter{}.exportToData(image);
    CORRADE_VERIFY(!data);
    CORRADE_COMPARE(out.str(), "Trade::JpegImageConverter::exportToData(): unsupported pixel format PixelFormat::DepthComponent\n");
}

void JpegImageConverterTest::wrongType() {
    ImageView2D image{PixelFormat::Red, PixelType::Float, {}, nullptr};

    std::ostringstream out;
    Error redirectError{&out};

    const auto data = JpegImageConverter{}.exportToData(image);
    CORRADE_VERIFY(!data);
    CORRADE_COMPARE(out.str(), "Trade::JpegImageConverter::exportToData(): cannot convert image of type PixelType::Float into 8-bit JPEG\n");
}

void JpegImageConverterTest::data() {
    const auto data = JpegImageConverter{}.exportToData(original);

    JpegImporter importer;
    CORRADE_VERIFY(importer.openData(data));
    std::optional<Trade::ImageData2D> converted = importer.image2D(0);
    CORRADE_VERIFY(converted);

    CORRADE_COMPARE(converted->size(), Vector2i(2, 3));
    CORRADE_COMPARE(converted->format(), PixelFormat::RGB);
    CORRADE_COMPARE(converted->type(), PixelType::UnsignedByte);

    /* The image has four-byte aligned rows, clear the padding to deterministic
       values */
    CORRADE_COMPARE(converted->data().size(), 24);
    converted->data()[6] = converted->data()[7] = converted->data()[14] =
         converted->data()[15] = converted->data()[22] = converted->data()[23] = 0;

    CORRADE_COMPARE_AS(converted->data(),
        Containers::ArrayView<const char>{ConvertedData},
        TestSuite::Compare::Container);
}

}}}

CORRADE_TEST_MAIN(Magnum::Trade::Test::JpegImageConverterTest)
