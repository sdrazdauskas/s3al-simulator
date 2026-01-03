#include <gtest/gtest.h>
#include "storage/Storage.h"
#include "testHelpers/MockSysApi.h"
#include "logger/Logger.h"

using namespace storage;
using Response = StorageManager::StorageResponse;

class StorageManagerTest : public ::testing::Test {
protected:
    testHelpers::MockSysApi mockSysApi;
    StorageManager storage;
    
    void SetUp() override {
        storage.setSysApi(&mockSysApi);
    }
};

// FILE CREATION, DELETION
TEST_F(StorageManagerTest, TouchFile_ShouldSucceedIfFileAlreadyExists) {
    EXPECT_EQ(storage.createFile("exists.txt"), Response::OK);
    EXPECT_EQ(storage.touchFile("exists.txt"), Response::OK);
}

TEST_F(StorageManagerTest, CreateFile_Success) {
    EXPECT_EQ(storage.createFile("file.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("file.txt"), Response::OK);
}

TEST_F(StorageManagerTest, CreateFile_AlreadyExists) {
    EXPECT_EQ(storage.createFile("dupe.txt"), Response::OK);
    EXPECT_EQ(storage.createFile("dupe.txt"), Response::AlreadyExists);
}

TEST_F(StorageManagerTest, CreateFile_InvalidName) {
    EXPECT_EQ(storage.createFile(""), Response::InvalidArgument);
    EXPECT_EQ(storage.createFile("   "), Response::InvalidArgument);
}

TEST_F(StorageManagerTest, CreateFile_AbsoluteFromNestedDirectory) {
    EXPECT_EQ(storage.makeDir("nested"), Response::OK);
    EXPECT_EQ(storage.changeDir("nested"), Response::OK);
    EXPECT_EQ(storage.createFile("/root_file.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("/root_file.txt"), Response::OK);
}

TEST_F(StorageManagerTest, DeleteFile_Success) {
    EXPECT_EQ(storage.createFile("tmp.txt"), Response::OK);
    EXPECT_EQ(storage.deleteFile("tmp.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("tmp.txt"), Response::NotFound);
}

TEST_F(StorageManagerTest, DeleteFile_InvalidOrMissing) {
    EXPECT_EQ(storage.deleteFile("ghost.txt"), Response::NotFound);
    EXPECT_EQ(storage.deleteFile(""), Response::InvalidArgument);
}

// FILE READ, WRITE, EDIT
TEST_F(StorageManagerTest, WriteAndReadFile_Success) {
    EXPECT_EQ(storage.createFile("data.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("data.txt", "hello"), Response::OK);

    std::string out;
    EXPECT_EQ(storage.readFile("data.txt", out), Response::OK);
    EXPECT_EQ(out, "hello\n");
}

TEST_F(StorageManagerTest, WriteFile_ShouldOverwriteExistingContent) {
    EXPECT_EQ(storage.createFile("test.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("test.txt", "v1"), Response::OK);
    EXPECT_EQ(storage.writeFile("test.txt", "v2"), Response::OK);

    std::string out;
    EXPECT_EQ(storage.readFile("test.txt", out), Response::OK);
    EXPECT_EQ(out, "v2\n");
}

TEST_F(StorageManagerTest, WriteFile_NotFound) {
    EXPECT_EQ(storage.writeFile("ghost.txt", "hi"), Response::NotFound);
}

TEST_F(StorageManagerTest, EditFile_AppendsContent) {
    EXPECT_EQ(storage.createFile("edit.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("edit.txt", "begin"), Response::OK);
    EXPECT_EQ(storage.editFile("edit.txt", " end"), Response::OK);

    std::string out;
    EXPECT_EQ(storage.readFile("edit.txt", out), Response::OK);
    EXPECT_EQ(out, "begin\n end");
}

TEST_F(StorageManagerTest, EditFile_NotFoundOrEmptyShouldFailGracefully) {
    EXPECT_EQ(storage.editFile("ghost.txt", "data"), Response::NotFound);
    EXPECT_EQ(storage.editFile("", "data"), Response::InvalidArgument);
}

TEST_F(StorageManagerTest, ReadFile_FromDirectory_ShouldReturnInvalidArgument) {
    EXPECT_EQ(storage.makeDir("mydir"), Response::OK);
    std::string out;
    EXPECT_EQ(storage.readFile("mydir", out), Response::NotFound);
}

// FILE COPY, MOVE
TEST_F(StorageManagerTest, CopyFile_Success) {
    EXPECT_EQ(storage.createFile("src.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("src.txt", "data"), Response::OK);
    EXPECT_EQ(storage.copyFile("src.txt", "copy.txt"), Response::OK);

    std::string a, b;
    EXPECT_EQ(storage.readFile("src.txt", a), Response::OK);
    EXPECT_EQ(storage.readFile("copy.txt", b), Response::OK);
    EXPECT_EQ(a, b);
}

TEST_F(StorageManagerTest, CopyFile_ShouldDuplicateFileWithDifferentName) {
    EXPECT_EQ(storage.createFile("a.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("a.txt", "hello"), Response::OK);
    EXPECT_EQ(storage.copyFile("a.txt", "b.txt"), Response::OK);

    std::string out1, out2;
    EXPECT_EQ(storage.readFile("a.txt", out1), Response::OK);
    EXPECT_EQ(storage.readFile("b.txt", out2), Response::OK);
    EXPECT_EQ(out1, out2);
}

TEST_F(StorageManagerTest, CopyFile_Errors) {
    EXPECT_EQ(storage.copyFile("missing.txt", "copy.txt"), Response::NotFound);
    EXPECT_EQ(storage.createFile("file.txt"), Response::OK);
    EXPECT_EQ(storage.createFile("file2.txt"), Response::OK);
    EXPECT_EQ(storage.copyFile("file.txt", "file2.txt"), Response::AlreadyExists);
}

TEST_F(StorageManagerTest, CopyFile_SamePathOrInvalidDestination) {
    EXPECT_EQ(storage.createFile("a.txt"), Response::OK);
    EXPECT_EQ(storage.copyFile("a.txt", "a.txt"), Response::AlreadyExists);
    EXPECT_EQ(storage.copyFile("", ""), Response::InvalidArgument);
}

TEST_F(StorageManagerTest, MoveFile_Success) {
    EXPECT_EQ(storage.createFile("move_me.txt"), Response::OK);
    EXPECT_EQ(storage.moveFile("move_me.txt", "moved.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("moved.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("move_me.txt"), Response::NotFound);
}

TEST_F(StorageManagerTest, MoveFile_ToSubdirectory) {
    EXPECT_EQ(storage.makeDir("target"), Response::OK);
    EXPECT_EQ(storage.createFile("item.txt"), Response::OK);
    EXPECT_EQ(storage.moveFile("item.txt", "target/item.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("target/item.txt"), Response::OK);
}

TEST_F(StorageManagerTest, MoveFile_ToSubdirectory2) {
    EXPECT_EQ(storage.makeDir("target"), Response::OK);
    EXPECT_EQ(storage.createFile("item.txt"), Response::OK);
    EXPECT_EQ(storage.moveFile("item.txt", "target"), Response::OK);
    EXPECT_EQ(storage.fileExists("target/item.txt"), Response::OK);
}

// DIRECTORY CREATION
TEST_F(StorageManagerTest, MakeDir_Success) {
    EXPECT_EQ(storage.makeDir("stuff"), Response::OK);
    EXPECT_EQ(storage.changeDir("stuff"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/stuff");
}

TEST_F(StorageManagerTest, MakeDir_InvalidOrNonexistentParent) {
    EXPECT_EQ(storage.makeDir(""), Response::InvalidArgument);
    EXPECT_EQ(storage.makeDir("a/b/c"), Response::NotFound);
}

TEST_F(StorageManagerTest, ChangeDir_BasicNavigation) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/b"), Response::OK);
    EXPECT_EQ(storage.changeDir("a/b"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/a/b");
    EXPECT_EQ(storage.changeDir(".."), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/a");
    EXPECT_EQ(storage.changeDir("/"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/");
}

TEST_F(StorageManagerTest, ChangeDir_PathNormalization) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/b"), Response::OK);
    EXPECT_EQ(storage.changeDir("a//b/."), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/a/b");
}

TEST_F(StorageManagerTest, ChangeDir_BeyondRoot_ShouldStayAtRoot) {
    EXPECT_EQ(storage.changeDir(".."), Response::AtRoot);
    EXPECT_EQ(storage.getWorkingDir(), "/");
}

// DIRECTORY COPY, MOVE
TEST_F(StorageManagerTest, CopyDir_SuccessRecursive) {
    EXPECT_EQ(storage.makeDir("src"), Response::OK);
    EXPECT_EQ(storage.createFile("src/data.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("src/data.txt", "content"), Response::OK);

    EXPECT_EQ(storage.copyDir("src", "backup"), Response::OK);

    std::string content;
    EXPECT_EQ(storage.readFile("backup/data.txt", content), Response::OK);
    EXPECT_EQ(content, "content\n");
}

TEST_F(StorageManagerTest, CopyDir_EdgeCases) {
    EXPECT_EQ(storage.makeDir("dir"), Response::OK);
    EXPECT_EQ(storage.makeDir("dir/dir2"), Response::OK);
    EXPECT_EQ(storage.copyDir("dir", "dir"), Response::OK);
    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("dir/dir", list), Response::OK);
    EXPECT_EQ(storage.listDir("dir/dir/dir2", list), Response::OK);
    EXPECT_EQ(storage.listDir("dir/dir2", list), Response::OK);
}

TEST_F(StorageManagerTest, CopyDir_ShouldCopyFolderWithContents) {
    EXPECT_EQ(storage.makeDir("srcdir"), Response::OK);
    EXPECT_EQ(storage.changeDir("srcdir"), Response::OK);
    EXPECT_EQ(storage.createFile("note.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("note.txt", "data"), Response::OK);
    EXPECT_EQ(storage.changeDir(".."), Response::OK);

    EXPECT_EQ(storage.copyDir("srcdir", "copydir"), Response::OK);

    EXPECT_EQ(storage.changeDir("copydir"), Response::OK);
    std::string out;
    EXPECT_EQ(storage.readFile("note.txt", out), Response::OK);
    EXPECT_EQ(out, "data\n");
}

TEST_F(StorageManagerTest, CopyDir_ShouldReturnAlreadyExistsIfSameNameInDest) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("b"), Response::OK);
    EXPECT_EQ(storage.changeDir("b"), Response::OK);
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.changeDir(".."), Response::OK);

    EXPECT_EQ(storage.copyDir("a", "b"), Response::AlreadyExists);
}

TEST_F(StorageManagerTest, MoveDir_SuccessAndContentPreserved) {
    EXPECT_EQ(storage.makeDir("source"), Response::OK);
    EXPECT_EQ(storage.createFile("source/file.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("source/file.txt", "x"), Response::OK);
    EXPECT_EQ(storage.moveDir("source", "moved"), Response::OK);
    EXPECT_EQ(storage.fileExists("moved/file.txt"), Response::OK);
}

TEST_F(StorageManagerTest, MoveDir_WithParentReferences) {
    EXPECT_EQ(storage.makeDir("parent"), Response::OK);
    EXPECT_EQ(storage.makeDir("parent/child"), Response::OK);
    EXPECT_EQ(storage.makeDir("parent/child/grandchild"), Response::OK);
    EXPECT_EQ(storage.changeDir("parent/child"), Response::OK);
    
    EXPECT_EQ(storage.moveDir("grandchild", "../moved"), Response::OK);
    
    EXPECT_EQ(storage.changeDir(".."), Response::OK);
    EXPECT_EQ(storage.changeDir("moved"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/parent/moved");
}

TEST_F(StorageManagerTest, MoveDir_BothPathsRelative) {
    EXPECT_EQ(storage.makeDir("old"), Response::OK);
    EXPECT_EQ(storage.makeDir("container"), Response::OK);
    EXPECT_EQ(storage.createFile("old/item.txt"), Response::OK);
    
    EXPECT_EQ(storage.moveDir("old", "container/new"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("old"), Response::NotFound);
    EXPECT_EQ(storage.changeDir("container/new"), Response::OK);
    EXPECT_EQ(storage.fileExists("item.txt"), Response::OK);
}

// LIST DIRECTORY

TEST_F(StorageManagerTest, ListDir_ShouldShowFilesAndDirs) {
    EXPECT_EQ(storage.makeDir("docs"), Response::OK);
    EXPECT_EQ(storage.createFile("a.txt"), Response::OK);

    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("", list), Response::OK);
    EXPECT_EQ(list.size(), 2);

    bool foundDir = false, foundFile = false;
    for (const auto& e : list) {
        if (e.rfind("[D]", 0) == 0) foundDir = true;
        if (e.rfind("[F]", 0) == 0) foundFile = true;
    }
    EXPECT_TRUE(foundDir);
    EXPECT_TRUE(foundFile);
}

TEST_F(StorageManagerTest, ListDir_EmptyDirectory) {
    EXPECT_EQ(storage.makeDir("empty"), Response::OK);
    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("empty", list), Response::OK);
    EXPECT_TRUE(list.empty());
}

TEST_F(StorageManagerTest, ListDir_InvalidPath_ShouldReturnNotFound) {
    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("ghost", list), Response::NotFound);
    EXPECT_TRUE(list.empty());
}

TEST_F(StorageManagerTest, ListDir_RelativePath_ShouldListChildItems) {
    EXPECT_EQ(storage.makeDir("folder"), Response::OK);
    EXPECT_EQ(storage.createFile("folder/a.txt"), Response::OK);
    EXPECT_EQ(storage.createFile("folder/b.txt"), Response::OK);

    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("folder", list), Response::OK);
    EXPECT_EQ(list.size(), 2);

    for (const auto& e : list) EXPECT_TRUE(e.rfind("[F]", 0) == 0);
}

TEST_F(StorageManagerTest, ListDir_AbsolutePath_ShouldWorkFromAnyPosition) {
    EXPECT_EQ(storage.makeDir("folder"), Response::OK);
    EXPECT_EQ(storage.createFile("folder/file.txt"), Response::OK);
    EXPECT_EQ(storage.changeDir("folder"), Response::OK);

    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("/folder", list), Response::OK);
    EXPECT_EQ(list.size(), 1);
    EXPECT_TRUE(list[0].find("file.txt") != std::string::npos);
}

TEST_F(StorageManagerTest, ListDir_DeepNestedPath) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/b"), Response::OK);
    EXPECT_EQ(storage.createFile("a/b/note.md"), Response::OK);

    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("a/b", list), Response::OK);
    ASSERT_EQ(list.size(), 1);
    EXPECT_TRUE(list[0].rfind("[F] note.md", 0) == 0);
}

TEST_F(StorageManagerTest, ListDir_RootDirectory) {
    EXPECT_EQ(storage.makeDir("folder1"), Response::OK);
    EXPECT_EQ(storage.createFile("main.txt"), Response::OK);

    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("/", list), Response::OK);
    EXPECT_EQ(list.size(), 2);
}

TEST_F(StorageManagerTest, ListDir_ShouldIncludeHiddenFiles) {
    EXPECT_EQ(storage.createFile(".hidden"), Response::OK);
    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("", list), Response::OK);

    bool foundHidden = false;
    for (auto& item : list)
        if (item.find(".hidden") != std::string::npos) foundHidden = true;

    EXPECT_TRUE(foundHidden);
}

// PATH RESOLUTION
TEST_F(StorageManagerTest, PathNormalization_ShouldCollapseExtraSlashes) {
    EXPECT_EQ(storage.makeDir("a/b/c"), Response::NotFound);
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/b"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/b/c"), Response::OK);
    EXPECT_EQ(storage.changeDir("a///b/./c"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/a/b/c");
}

TEST_F(StorageManagerTest, RelativeAndAbsolutePaths_ReferSameObject) {
    EXPECT_EQ(storage.makeDir("docs"), Response::OK);
    EXPECT_EQ(storage.createFile("/docs/file.txt"), Response::OK);
    EXPECT_EQ(storage.changeDir("/"), Response::OK);
    std::string outAbs, outRel;
    EXPECT_EQ(storage.readFile("/docs/file.txt", outAbs), Response::OK);
    EXPECT_EQ(storage.readFile("docs/file.txt", outRel), Response::OK);
    EXPECT_EQ(outAbs, outRel);
}

TEST_F(StorageManagerTest, ComplexPath_NavigateWithParentReferences) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/b"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/b/c"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/x"), Response::OK);
    EXPECT_EQ(storage.changeDir("a/b/c"), Response::OK);
    
    EXPECT_EQ(storage.createFile("../../x/file.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("../../x/file.txt"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("/a/x"), Response::OK);
    EXPECT_EQ(storage.fileExists("file.txt"), Response::OK);
}

TEST_F(StorageManagerTest, ComplexPath_MixedAbsoluteRelativeOperations) {
    EXPECT_EQ(storage.makeDir("/home"), Response::OK);
    EXPECT_EQ(storage.changeDir("/home"), Response::OK);
    EXPECT_EQ(storage.makeDir("user"), Response::OK);
    EXPECT_EQ(storage.makeDir("/home/user/documents"), Response::OK);
    EXPECT_EQ(storage.createFile("user/documents/note.txt"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("/"), Response::OK);
    EXPECT_EQ(storage.fileExists("home/user/documents/note.txt"), Response::OK);
}

TEST_F(StorageManagerTest, ComplexPath_CreateAndAccessNestedStructure) {
    EXPECT_EQ(storage.makeDir("workspace"), Response::OK);
    EXPECT_EQ(storage.makeDir("workspace/project"), Response::OK);
    EXPECT_EQ(storage.makeDir("workspace/project/src"), Response::OK);
    EXPECT_EQ(storage.createFile("workspace/project/src/main.cpp"), Response::OK);
    EXPECT_EQ(storage.writeFile("workspace/project/src/main.cpp", "int main()"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("workspace"), Response::OK);
    std::string content;
    EXPECT_EQ(storage.readFile("project/src/main.cpp", content), Response::OK);
    EXPECT_EQ(content, "int main()\n");
}

TEST_F(StorageManagerTest, ChangeDir_WithParentReferences) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/b"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/c"), Response::OK);
    EXPECT_EQ(storage.changeDir("a/b"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("../c"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/a/c");
}

TEST_F(StorageManagerTest, ChangeDir_WithAbsolutePath) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/b"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/b/c"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("/a/b/c"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/a/b/c");
}

// ELSE
TEST_F(StorageManagerTest, FileLifecycle_EndToEnd) {
    EXPECT_EQ(storage.createFile("story.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("story.txt", "Chapter 1"), Response::OK);
    EXPECT_EQ(storage.editFile("story.txt", "\nChapter 2"), Response::OK);
    EXPECT_EQ(storage.copyFile("story.txt", "copy.txt"), Response::OK);
    EXPECT_EQ(storage.moveFile("copy.txt", "final.txt"), Response::OK);
    EXPECT_EQ(storage.deleteFile("story.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("final.txt"), Response::OK);
}

TEST_F(StorageManagerTest, DirectoryLifecycle_EndToEnd) {
    EXPECT_EQ(storage.makeDir("projects"), Response::OK);
    EXPECT_EQ(storage.createFile("projects/code.cpp"), Response::OK);
    EXPECT_EQ(storage.writeFile("projects/code.cpp", "int main()"), Response::OK);
    EXPECT_EQ(storage.copyDir("projects", "backup"), Response::OK);
    EXPECT_EQ(storage.moveDir("backup", "final_backup"), Response::OK);
    EXPECT_EQ(storage.removeDir("projects"), Response::OK);
    EXPECT_EQ(storage.fileExists("projects"), Response::NotFound);
}

TEST_F(StorageManagerTest, ChangeDir_AttemptToEscapeRoot_ShouldStayAtRoot) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.changeDir("../../.."), Response::AtRoot);
    EXPECT_EQ(storage.getWorkingDir(), "/");
}

TEST_F(StorageManagerTest, PathOperations_EmptyPathReturnsInvalidArgument) {
    EXPECT_EQ(storage.createFile(""), Response::InvalidArgument);
    EXPECT_EQ(storage.makeDir(""), Response::InvalidArgument);
    EXPECT_EQ(storage.deleteFile(""), Response::InvalidArgument);
    EXPECT_EQ(storage.removeDir(""), Response::InvalidArgument);
}
