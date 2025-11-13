#include <gtest/gtest.h>
#include "Storage.h"

using namespace storage;
using Response = StorageManager::StorageResponse;

class StorageManagerTest : public ::testing::Test {
protected:
    StorageManager storage;
};



// File Tests
TEST_F(StorageManagerTest, CreateFile_ShouldBeListedAfterCreation) {
    EXPECT_EQ(storage.createFile("doc.txt"), Response::OK);
    const auto entries = storage.listDir();
    ASSERT_EQ(entries.size(), 1);
    EXPECT_TRUE(entries[0].find("doc.txt") != std::string::npos);
}

TEST_F(StorageManagerTest, WriteFile_ShouldSaveContentWithNewline) {
    EXPECT_EQ(storage.createFile("data.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("data.txt", "hello"), Response::OK);

    std::string out;
    EXPECT_EQ(storage.readFile("data.txt", out), Response::OK);
    EXPECT_EQ(out, "hello\n");
}

TEST_F(StorageManagerTest, DeleteFile_ShouldRemoveItFromDirectory) {
    EXPECT_EQ(storage.createFile("tmp.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("tmp.txt", "test"), Response::OK);

    EXPECT_EQ(storage.deleteFile("tmp.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("tmp.txt"), Response::NotFound);
}

TEST_F(StorageManagerTest, CreateFile_Success) {
    EXPECT_EQ(storage.createFile("file1.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("file1.txt"), Response::OK);
}

TEST_F(StorageManagerTest, CreateFile_AlreadyExists) {
    storage.createFile("dupe.txt");
    EXPECT_EQ(storage.createFile("dupe.txt"), Response::AlreadyExists);
}

TEST_F(StorageManagerTest, CreateFile_InvalidName) {
    EXPECT_EQ(storage.createFile(""), Response::InvalidArgument);
    EXPECT_EQ(storage.createFile("    "), Response::InvalidArgument);
}

TEST_F(StorageManagerTest, WriteAndReadFile_Success) {
    storage.createFile("data.txt");
    EXPECT_EQ(storage.writeFile("data.txt", "hello"), Response::OK);

    std::string contents;
    EXPECT_EQ(storage.readFile("data.txt", contents), Response::OK);
    EXPECT_EQ(contents, "hello\n");
}

TEST_F(StorageManagerTest, WriteFile_NotFound) {
    EXPECT_EQ(storage.writeFile("ghost.txt", "hi"), Response::NotFound);
}

TEST_F(StorageManagerTest, EditFile_ShouldAppendNewLines) {
    EXPECT_EQ(storage.createFile("test.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("test.txt", "test"), Response::OK);

    const std::string newContent = "foo\nbar\n";

    const auto response = storage.editFile("test.txt", newContent);
    EXPECT_EQ(response, StorageManager::StorageResponse::OK);

    std::string contents;
    EXPECT_EQ(storage.readFile("test.txt", contents), Response::OK);
    EXPECT_EQ(contents, "test\nfoo\nbar\n");
}

TEST_F(StorageManagerTest, EditFile_ShouldReturnNotFoundForMissingFile) {
    const std::string newContent = "some text\n";
    const auto response = storage.editFile("ghost.txt", newContent);
    EXPECT_EQ(response, StorageManager::StorageResponse::NotFound);
}

TEST_F(StorageManagerTest, EditFile_ShouldReturnInvalidArgumentForEmptyName) {
    const std::string newContent = "anything\n";
    const auto response = storage.editFile("", newContent);
    EXPECT_EQ(response, StorageManager::StorageResponse::InvalidArgument);
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

TEST_F(StorageManagerTest, CopyFile_ShouldFailIfSourceDoesNotExist) {
    EXPECT_EQ(storage.copyFile("ghost.txt", "new.txt"), Response::NotFound);
}

TEST_F(StorageManagerTest, CopyFile_ShouldReturnAlreadyExistsIfDestExists) {
    EXPECT_EQ(storage.createFile("src.txt"), Response::OK);
    EXPECT_EQ(storage.createFile("dest.txt"), Response::OK);
    EXPECT_EQ(storage.copyFile("src.txt", "dest.txt"), Response::AlreadyExists);
}

TEST_F(StorageManagerTest, MoveFile_ShouldRenameFileInSameDirectory) {
    EXPECT_EQ(storage.createFile("oldname.txt"), Response::OK);
    EXPECT_EQ(storage.moveFile("oldname.txt", "newname.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("newname.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("oldname.txt"), Response::NotFound);
}

TEST_F(StorageManagerTest, MoveFile_ShouldMoveIntoExistingDirectory) {
    EXPECT_EQ(storage.makeDir("target"), Response::OK);
    EXPECT_EQ(storage.createFile("move_me.txt"), Response::OK);
    EXPECT_EQ(storage.moveFile("move_me.txt", "target"), Response::OK);

    EXPECT_EQ(storage.changeDir("target"), Response::OK);
    EXPECT_EQ(storage.fileExists("move_me.txt"), Response::OK);
    EXPECT_EQ(storage.changeDir(".."), Response::OK);
    EXPECT_EQ(storage.fileExists("move_me.txt"), Response::NotFound);
}

TEST_F(StorageManagerTest, MoveFile_ShouldReturnNotFoundIfSourceMissing) {
    EXPECT_EQ(storage.moveFile("missing.txt", "whatever"), Response::NotFound);
}

// Directory Tests
TEST_F(StorageManagerTest, MakeAndChangeDir_Success) {
    EXPECT_EQ(storage.makeDir("stuff"), Response::OK);
    EXPECT_EQ(storage.changeDir("stuff"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/root/stuff");
    EXPECT_EQ(storage.changeDir(".."), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/root");
}

TEST_F(StorageManagerTest, ChangeDir_AtRoot_ReturnsAtRoot) {
    EXPECT_EQ(storage.changeDir(".."), Response::AtRoot);
}

TEST_F(StorageManagerTest, MakeDir_InvalidName_ShouldReturnInvalidArgument) {
    EXPECT_EQ(storage.makeDir(" "), Response::InvalidArgument);
}

TEST_F(StorageManagerTest, RemoveDir_SuccessfullyRemovesIt) {
    storage.makeDir("trash");
    EXPECT_EQ(storage.removeDir("trash"), Response::OK);
    EXPECT_EQ(storage.changeDir("trash"), Response::NotFound);
}

TEST_F(StorageManagerTest, ListDir_ShouldReturnFilesAndDirs) {
    storage.createFile("a.txt");
    storage.makeDir("docs");
    const auto list = storage.listDir();
    EXPECT_EQ(list.size(), 2);
    EXPECT_TRUE(list[0].rfind("[D]", 0) == 0 || list[1].rfind("[D]", 0) == 0);
    EXPECT_TRUE(list[0].rfind("[F]", 0) == 0 || list[1].rfind("[F]", 0) == 0);
}

TEST_F(StorageManagerTest, RemoveDir_ShouldDeleteAllNestedFilesAndFolders) {
    EXPECT_EQ(storage.makeDir("parent"), Response::OK);
    EXPECT_EQ(storage.changeDir("parent"), Response::OK);

    EXPECT_EQ(storage.createFile("rootfile.txt"), Response::OK);
    EXPECT_EQ(storage.makeDir("child"), Response::OK);
    EXPECT_EQ(storage.changeDir("child"), Response::OK);
    EXPECT_EQ(storage.createFile("nested.txt"), Response::OK);

    EXPECT_EQ(storage.changeDir(".."), Response::OK);
    EXPECT_EQ(storage.changeDir(".."), Response::OK);

    auto before = storage.listDir();
    ASSERT_EQ(before.size(), 1);
    EXPECT_TRUE(before[0].find("parent") != std::string::npos);

    EXPECT_EQ(storage.removeDir("parent"), Response::OK);

    auto after = storage.listDir();
    EXPECT_TRUE(after.empty());

    EXPECT_EQ(storage.changeDir("parent"), Response::NotFound);
    EXPECT_EQ(storage.changeDir("child"), Response::NotFound);
    EXPECT_EQ(storage.writeFile("rootfile.txt", "hi"), Response::NotFound);
    EXPECT_EQ(storage.writeFile("nested.txt", "hi"), Response::NotFound);
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

TEST_F(StorageManagerTest, CopyDir_ShouldCopyIntoExistingDirectory) {
    EXPECT_EQ(storage.makeDir("srcdir"), Response::OK);
    EXPECT_EQ(storage.makeDir("destdir"), Response::OK);

    EXPECT_EQ(storage.changeDir("srcdir"), Response::OK);
    EXPECT_EQ(storage.createFile("inside.txt"), Response::OK);
    EXPECT_EQ(storage.changeDir(".."), Response::OK);

    EXPECT_EQ(storage.copyDir("srcdir", "destdir"), Response::OK);

    EXPECT_EQ(storage.changeDir("destdir"), Response::OK);
    EXPECT_EQ(storage.changeDir("srcdir"), Response::OK);
    EXPECT_EQ(storage.fileExists("inside.txt"), Response::OK);
}

TEST_F(StorageManagerTest, MoveDir_ShouldRenameDirectory) {
    EXPECT_EQ(storage.makeDir("rename_me"), Response::OK);
    EXPECT_EQ(storage.moveDir("rename_me", "renamed"), Response::OK);
    EXPECT_EQ(storage.changeDir("rename_me"), Response::NotFound);
    EXPECT_EQ(storage.changeDir("renamed"), Response::OK);
}

TEST_F(StorageManagerTest, MoveDir_ShouldReturnNotFoundIfSourceMissing) {
    EXPECT_EQ(storage.moveDir("ghostdir", "whatever"), Response::NotFound);
}

TEST_F(StorageManagerTest, CopyDir_ShouldReturnNotFoundIfMissingSource) {
    EXPECT_EQ(storage.copyDir("ghostdir", "whatever"), Response::NotFound);
}

TEST_F(StorageManagerTest, CopyDir_ShouldReturnAlreadyExistsIfSameNameInDest) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("b"), Response::OK);
    EXPECT_EQ(storage.changeDir("b"), Response::OK);
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.changeDir(".."), Response::OK);

    EXPECT_EQ(storage.copyDir("a", "b"), Response::AlreadyExists);
}
