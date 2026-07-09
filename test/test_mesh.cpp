#include <gtest/gtest.h>
#include "Forge/Mesh.hpp"

#include <sstream>

namespace
{
    // Every test below builds a tiny in-memory OBJ file instead of a real one on disk -
    // ParseObjFile only needs a stream, so a plain string literal is enough.
    Forge::ParsedMeshData ParseObjString(const std::string &objText, const std::string &sourceName = "test.obj")
    {
        std::istringstream stream(objText);
        return Forge::Mesh::ParseObjFile(stream, sourceName);
    }

    void ExpectVec3Eq(const glm::vec3 &actual, const glm::vec3 &expected)
    {
        EXPECT_FLOAT_EQ(actual.x, expected.x);
        EXPECT_FLOAT_EQ(actual.y, expected.y);
        EXPECT_FLOAT_EQ(actual.z, expected.z);
    }

    void ExpectVec2Eq(const glm::vec2 &actual, const glm::vec2 &expected)
    {
        EXPECT_FLOAT_EQ(actual.x, expected.x);
        EXPECT_FLOAT_EQ(actual.y, expected.y);
    }
}

TEST(MeshTest, CreateObjFromFile)
{
    std::ifstream crateFile("models/crate.obj");
    auto crate1 = Forge::Mesh::ParseObjFile(crateFile, "models/crate.obj");

    crateFile.clear();
    crateFile.seekg(0);
    auto crate2 = Forge::Mesh::ParseObjFile(crateFile, "models/crate.obj");

    EXPECT_EQ(crate1, crate2);
    EXPECT_EQ(crate1.indices, crate2.indices);
    EXPECT_EQ(crate1.vertices, crate2.vertices);

    std::ifstream starFile("models/star.obj");
    auto star = Forge::Mesh::ParseObjFile(starFile, "models/star.obj");

    EXPECT_NE(star, crate1);
    EXPECT_NE(star, crate2);
}

// --- Face-index forms -------------------------------------------------------------------
//
// An OBJ "f" line can reference position/texcoord/normal in four different textual shapes
// ("1", "1/1", "1/1/1", "1//1"), each parsed by a different sscanf pattern - four genuinely
// separate code paths, so each gets its own test. Most of these include a "vn" line that the
// face tokens never actually reference; its only job is to keep `normals` non-empty so flat
// normal generation (a separate branch, tested further down) doesn't run and overwrite the
// vertex data being inspected here.

TEST(MeshTest, FaceFormVOnlyParsesPositionOnly)
{
    auto result = ParseObjString(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "vn 0 0 1\n"
        "f 1 2 3\n");

    ASSERT_EQ(result.vertices.size(), 3u);
    ExpectVec3Eq(result.vertices[0].position, glm::vec3(0.0f, 0.0f, 0.0f));
    ExpectVec3Eq(result.vertices[1].position, glm::vec3(1.0f, 0.0f, 0.0f));
    ExpectVec3Eq(result.vertices[2].position, glm::vec3(0.0f, 1.0f, 0.0f));
    // "v"-only tokens never reference a texcoord/normal index, so both stay at Vertex{}'s
    // zero default (confirmed separately: GLM's vec() is "= default", so value-initializing
    // Vertex{} genuinely zeroes its glm members rather than leaving them uninitialized).
    for (const auto &vertex : result.vertices)
    {
        ExpectVec2Eq(vertex.texCoords, glm::vec2(0.0f, 0.0f));
        ExpectVec3Eq(vertex.normal, glm::vec3(0.0f, 0.0f, 0.0f));
    }
}

TEST(MeshTest, FaceFormVSlashVtParsesTexCoord)
{
    // "vt" v-coordinates are flipped on load ("flip Y for OpenGL" in ParseObjFile), so
    // "vt 0 0" becomes (0,1) here, not (0,0) - keep that in mind reading the expected values.
    auto result = ParseObjString(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "vt 0 0\n"
        "vt 1 0\n"
        "vt 0 1\n"
        "vn 0 0 1\n"
        "f 1/1 2/2 3/3\n");

    ASSERT_EQ(result.vertices.size(), 3u);
    ExpectVec2Eq(result.vertices[0].texCoords, glm::vec2(0.0f, 1.0f));
    ExpectVec2Eq(result.vertices[1].texCoords, glm::vec2(1.0f, 1.0f));
    ExpectVec2Eq(result.vertices[2].texCoords, glm::vec2(0.0f, 0.0f));
    // "v/vt" tokens never reference a normal index.
    for (const auto &vertex : result.vertices)
        ExpectVec3Eq(vertex.normal, glm::vec3(0.0f, 0.0f, 0.0f));
}

TEST(MeshTest, FaceFormVSlashVtSlashVnParsesAll)
{
    auto result = ParseObjString(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "vt 0 0\n"
        "vt 1 0\n"
        "vt 0 1\n"
        "vn 0 0 1\n"
        "vn 0 1 0\n"
        "vn 1 0 0\n"
        "f 1/1/1 2/2/2 3/3/3\n");

    ASSERT_EQ(result.vertices.size(), 3u);
    ExpectVec3Eq(result.vertices[0].position, glm::vec3(0.0f, 0.0f, 0.0f));
    ExpectVec2Eq(result.vertices[0].texCoords, glm::vec2(0.0f, 1.0f));
    ExpectVec3Eq(result.vertices[0].normal, glm::vec3(0.0f, 0.0f, 1.0f));
    ExpectVec3Eq(result.vertices[1].normal, glm::vec3(0.0f, 1.0f, 0.0f));
    ExpectVec3Eq(result.vertices[2].normal, glm::vec3(1.0f, 0.0f, 0.0f));
}

TEST(MeshTest, FaceFormVSlashSlashVnSkipsTexCoord)
{
    auto result = ParseObjString(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "vn 0 0 1\n"
        "vn 0 1 0\n"
        "vn 1 0 0\n"
        "f 1//1 2//2 3//3\n");

    ASSERT_EQ(result.vertices.size(), 3u);
    ExpectVec3Eq(result.vertices[0].normal, glm::vec3(0.0f, 0.0f, 1.0f));
    ExpectVec3Eq(result.vertices[1].normal, glm::vec3(0.0f, 1.0f, 0.0f));
    ExpectVec3Eq(result.vertices[2].normal, glm::vec3(1.0f, 0.0f, 0.0f));
    // "v//vn" tokens never reference a texcoord index.
    for (const auto &vertex : result.vertices)
        ExpectVec2Eq(vertex.texCoords, glm::vec2(0.0f, 0.0f));
}

// --- Face shape and error handling ------------------------------------------------------

// A face with more than 3 vertices is fan-triangulated, anchored at its first vertex: a quad
// (4 tokens) must become exactly 2 triangles sharing that anchor - (0,1,2) and (0,2,3) - not
// 1 triangle and not 3.
TEST(MeshTest, QuadFaceFanTriangulatesIntoTwoTriangles)
{
    auto result = ParseObjString(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 1 1 0\n"
        "v 0 1 0\n"
        "vn 0 0 1\n"
        "f 1 2 3 4\n");

    ASSERT_EQ(result.vertices.size(), 4u);
    std::vector<unsigned int> expectedIndices = {0, 1, 2, 0, 2, 3};
    EXPECT_EQ(result.indices, expectedIndices);
}

// A face line with fewer than 3 vertices is skipped with a warning, not thrown - and parsing
// must continue normally afterward, not abort the whole file.
TEST(MeshTest, DegenerateFaceIsSkippedButParsingContinues)
{
    auto result = ParseObjString(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "v 5 5 5\n"
        "vn 0 0 1\n"
        "f 1 4\n" // degenerate: only 2 vertices, must be skipped, not crash
        "f 1//1 2//1 3//1\n");

    ASSERT_EQ(result.vertices.size(), 3u); // the degenerate face's "v 5 5 5" is never referenced
    std::vector<unsigned int> expectedIndices = {0, 1, 2};
    EXPECT_EQ(result.indices, expectedIndices);
}

// Real OBJ files carry keywords this parser doesn't implement (o/g/s/usemtl/mtllib). None of
// them match "v"/"vn"/"vt"/"f", so they must fall through silently rather than corrupt parsing.
TEST(MeshTest, UnknownKeywordLinesAreIgnored)
{
    auto result = ParseObjString(
        "o TestObject\n"
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "vn 0 0 1\n"
        "g group1\n"
        "s 1\n"
        "usemtl SomeMaterial\n"
        "mtllib scene.mtl\n"
        "f 1//1 2//1 3//1\n");

    ASSERT_EQ(result.vertices.size(), 3u);
    for (const auto &vertex : result.vertices)
        ExpectVec3Eq(vertex.normal, glm::vec3(0.0f, 0.0f, 1.0f));
}

// Out-of-range indices are handled inconsistently: an out-of-range position index warns and
// defaults to the origin; an out-of-range texcoord or normal index is silently left at its
// default with no warning at all. Neither one crashes - that's the actual guarantee worth
// pinning down here, the inconsistency is just what makes it worth knowing precisely.
TEST(MeshTest, OutOfRangeIndicesDefaultWithoutCrashing)
{
    auto result = ParseObjString(
        "v 2 2 2\n"
        "v 3 3 3\n"
        "vt 0 0\n"
        "vn 0 0 1\n"
        "f 99/1/1 1/99/1 2/1/99\n");

    ASSERT_EQ(result.vertices.size(), 3u);
    // "99/1/1": position index out of range -> defaults to the origin.
    ExpectVec3Eq(result.vertices[0].position, glm::vec3(0.0f, 0.0f, 0.0f));
    ExpectVec2Eq(result.vertices[0].texCoords, glm::vec2(0.0f, 1.0f));
    ExpectVec3Eq(result.vertices[0].normal, glm::vec3(0.0f, 0.0f, 1.0f));
    // "1/99/1": texcoord index out of range -> defaults to (0,0).
    ExpectVec3Eq(result.vertices[1].position, glm::vec3(2.0f, 2.0f, 2.0f));
    ExpectVec2Eq(result.vertices[1].texCoords, glm::vec2(0.0f, 0.0f));
    // "2/1/99": normal index out of range -> defaults to (0,0,0).
    ExpectVec3Eq(result.vertices[2].position, glm::vec3(3.0f, 3.0f, 3.0f));
    ExpectVec3Eq(result.vertices[2].normal, glm::vec3(0.0f, 0.0f, 0.0f));
}

// --- Flat normal generation branch ------------------------------------------------------
//
// These two tests share identical geometry - two triangles fanning a quad, sharing an edge -
// and differ only in whether the file has any "vn" line at all, to make the branch's effect
// directly comparable: 4 shared vertices with normals present, 6 unshared ones without.

// With vn data present, flat normal generation is skipped, so the shared edge (tokens "1"
// and "3", reused by both faces) is deduplicated: 4 unique vertices for 2 triangles, not 6.
TEST(MeshTest, NormalsPresentPreservesSharedVertices)
{
    auto result = ParseObjString(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 1 1 0\n"
        "v 0 1 0\n"
        "vn 0 0 1\n"
        "f 1 2 3\n"
        "f 1 3 4\n");

    EXPECT_EQ(result.vertices.size(), 4u);
    std::vector<unsigned int> expectedIndices = {0, 1, 2, 0, 2, 3};
    EXPECT_EQ(result.indices, expectedIndices);
}

// Same geometry, no "vn" line at all: flat normal generation now runs, expanding every
// triangle to its own unshared vertices (6, not 4) and computing a per-face normal via cross
// product. Both triangles of this flat quad face +Z, so both end up with the same normal.
TEST(MeshTest, NoNormalsExpandsToUnsharedFlatVertices)
{
    auto result = ParseObjString(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 1 1 0\n"
        "v 0 1 0\n"
        "f 1 2 3\n"
        "f 1 3 4\n");

    ASSERT_EQ(result.vertices.size(), 6u);
    for (const auto &vertex : result.vertices)
        ExpectVec3Eq(vertex.normal, glm::vec3(0.0f, 0.0f, 1.0f));
}

TEST(MeshTest, EmptyInputThrows)
{
    EXPECT_THROW(ParseObjString(""), std::runtime_error);
}
