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
    std::istringstream fakeInput("foo\nbar\n:wq\n");
    std::ostringstream fakeOutput;

    std::streambuf* originalCin = std::cin.rdbuf(fakeInput.rdbuf());
    std::streambuf* originalCout = std::cout.rdbuf(fakeOutput.rdbuf());

    EXPECT_EQ(storage.createFile("test.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("test.txt", "test"), Response::OK);

    const auto response = storage.editFile("test.txt");

    std::cin.rdbuf(originalCin);
    std::cout.rdbuf(originalCout);

    EXPECT_EQ(response, StorageManager::StorageResponse::OK);

    std::string contents;
    EXPECT_EQ(storage.readFile("test.txt", contents), Response::OK);
    EXPECT_EQ(contents, "test\nfoo\nbar\n");
}

TEST_F(StorageManagerTest, EditFile_ShouldReturnNotFoundForMissingFile) {
    std::istringstream fakeInput(":wq\n");
    std::streambuf* originalCin = std::cin.rdbuf(fakeInput.rdbuf());

    const auto response = storage.editFile("ghost.txt");
    std::cin.rdbuf(originalCin);

    EXPECT_EQ(response, StorageManager::StorageResponse::NotFound);
}

TEST_F(StorageManagerTest, EditFile_ShouldReturnInvalidArgumentForEmptyName) {
    const auto response = storage.editFile("");
    EXPECT_EQ(response, StorageManager::StorageResponse::InvalidArgument);
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
