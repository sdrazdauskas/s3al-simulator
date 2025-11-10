#include <gtest/gtest.h>
#include "Storage.h"

using namespace storage;
using Response = StorageManager::StorageResponse;

class StorageManagerTest : public ::testing::Test {
protected:
    StorageManager storage;
};



// FILE TESTS
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
    EXPECT_EQ(storage.moveFile("move_me.txt", "target/move_me.txt"), Response::OK);

    EXPECT_EQ(storage.changeDir("target"), Response::OK);
    EXPECT_EQ(storage.fileExists("move_me.txt"), Response::OK);
    EXPECT_EQ(storage.changeDir(".."), Response::OK);
    EXPECT_EQ(storage.fileExists("move_me.txt"), Response::NotFound);
}

TEST_F(StorageManagerTest, MoveFile_ShouldReturnNotFoundIfSourceMissing) {
    EXPECT_EQ(storage.moveFile("missing.txt", "whatever"), Response::NotFound);
}

// FILE TESTS WITH PATHS
TEST_F(StorageManagerTest, CreateFile_WithRelativePath) {
    EXPECT_EQ(storage.makeDir("docs"), Response::OK);
    EXPECT_EQ(storage.createFile("docs/readme.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("docs/readme.txt"), Response::OK);
}

TEST_F(StorageManagerTest, CreateFile_WithDeepRelativePath) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.changeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("b"), Response::OK);
    EXPECT_EQ(storage.changeDir("b"), Response::OK);
    EXPECT_EQ(storage.makeDir("c"), Response::OK);
    EXPECT_EQ(storage.changeDir(".."), Response::OK);
    EXPECT_EQ(storage.changeDir(".."), Response::OK);
    
    EXPECT_EQ(storage.createFile("a/b/c/deep.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("a/b/c/deep.txt"), Response::OK);
}

TEST_F(StorageManagerTest, CreateFile_WithAbsolutePath) {
    EXPECT_EQ(storage.makeDir("folder"), Response::OK);
    EXPECT_EQ(storage.changeDir("folder"), Response::OK);
    
    EXPECT_EQ(storage.createFile("/test.txt"), Response::OK);
    EXPECT_EQ(storage.changeDir("/"), Response::OK);
    EXPECT_EQ(storage.fileExists("test.txt"), Response::OK);
}

TEST_F(StorageManagerTest, TouchFile_WithRelativePath) {
    EXPECT_EQ(storage.makeDir("tmp"), Response::OK);
    EXPECT_EQ(storage.touchFile("tmp/file.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("tmp/file.txt"), Response::OK);
}

TEST_F(StorageManagerTest, WriteFile_WithRelativePath) {
    EXPECT_EQ(storage.makeDir("data"), Response::OK);
    EXPECT_EQ(storage.createFile("data/log.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("data/log.txt", "entry1"), Response::OK);
    
    std::string content;
    EXPECT_EQ(storage.readFile("data/log.txt", content), Response::OK);
    EXPECT_EQ(content, "entry1\n");
}

TEST_F(StorageManagerTest, WriteFile_WithAbsolutePath) {
    EXPECT_EQ(storage.makeDir("logs"), Response::OK);
    EXPECT_EQ(storage.createFile("logs/app.log"), Response::OK);
    EXPECT_EQ(storage.changeDir("logs"), Response::OK);
    
    EXPECT_EQ(storage.writeFile("/logs/app.log", "error"), Response::OK);
    
    std::string content;
    EXPECT_EQ(storage.readFile("app.log", content), Response::OK);
    EXPECT_EQ(content, "error\n");
}

TEST_F(StorageManagerTest, ReadFile_WithRelativePath) {
    EXPECT_EQ(storage.makeDir("docs"), Response::OK);
    EXPECT_EQ(storage.createFile("docs/readme.md"), Response::OK);
    EXPECT_EQ(storage.writeFile("docs/readme.md", "# Hello"), Response::OK);
    
    std::string content;
    EXPECT_EQ(storage.readFile("docs/readme.md", content), Response::OK);
    EXPECT_EQ(content, "# Hello\n");
}

TEST_F(StorageManagerTest, ReadFile_WithAbsolutePath) {
    EXPECT_EQ(storage.makeDir("config"), Response::OK);
    EXPECT_EQ(storage.createFile("config/app.conf"), Response::OK);
    EXPECT_EQ(storage.writeFile("config/app.conf", "port=8080"), Response::OK);
    EXPECT_EQ(storage.changeDir("config"), Response::OK);
    
    std::string content;
    EXPECT_EQ(storage.readFile("/config/app.conf", content), Response::OK);
    EXPECT_EQ(content, "port=8080\n");
}

TEST_F(StorageManagerTest, DeleteFile_WithRelativePath) {
    EXPECT_EQ(storage.makeDir("temp"), Response::OK);
    EXPECT_EQ(storage.createFile("temp/delete_me.txt"), Response::OK);
    EXPECT_EQ(storage.deleteFile("temp/delete_me.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("temp/delete_me.txt"), Response::NotFound);
}

TEST_F(StorageManagerTest, DeleteFile_WithAbsolutePath) {
    EXPECT_EQ(storage.makeDir("trash"), Response::OK);
    EXPECT_EQ(storage.createFile("trash/old.txt"), Response::OK);
    EXPECT_EQ(storage.changeDir("trash"), Response::OK);
    
    EXPECT_EQ(storage.deleteFile("/trash/old.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("old.txt"), Response::NotFound);
}

TEST_F(StorageManagerTest, EditFile_WithRelativePath) {
    EXPECT_EQ(storage.makeDir("notes"), Response::OK);
    EXPECT_EQ(storage.createFile("notes/todo.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("notes/todo.txt", "task1"), Response::OK);
    EXPECT_EQ(storage.editFile("notes/todo.txt", "\ntask2"), Response::OK);
    
    std::string content;
    EXPECT_EQ(storage.readFile("notes/todo.txt", content), Response::OK);
    EXPECT_EQ(content, "task1\n\ntask2");
}

TEST_F(StorageManagerTest, CopyFile_BothPathsRelative) {
    EXPECT_EQ(storage.makeDir("src"), Response::OK);
    EXPECT_EQ(storage.makeDir("dest"), Response::OK);
    EXPECT_EQ(storage.createFile("src/file.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("src/file.txt", "data"), Response::OK);
    
    EXPECT_EQ(storage.copyFile("src/file.txt", "dest/copy.txt"), Response::OK);
    
    std::string content;
    EXPECT_EQ(storage.readFile("dest/copy.txt", content), Response::OK);
    EXPECT_EQ(content, "data\n");
}

TEST_F(StorageManagerTest, CopyFile_SourceRelativeDestAbsolute) {
    EXPECT_EQ(storage.makeDir("folder1"), Response::OK);
    EXPECT_EQ(storage.makeDir("folder2"), Response::OK);
    EXPECT_EQ(storage.createFile("folder1/orig.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("folder1/orig.txt", "content"), Response::OK);
    EXPECT_EQ(storage.changeDir("folder1"), Response::OK);
    
    EXPECT_EQ(storage.copyFile("orig.txt", "/folder2/copy.txt"), Response::OK);
    
    std::string content;
    EXPECT_EQ(storage.readFile("/folder2/copy.txt", content), Response::OK);
    EXPECT_EQ(content, "content\n");
}

TEST_F(StorageManagerTest, CopyFile_BothPathsAbsolute) {
    EXPECT_EQ(storage.makeDir("dir1"), Response::OK);
    EXPECT_EQ(storage.makeDir("dir2"), Response::OK);
    EXPECT_EQ(storage.createFile("dir1/source.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("dir1/source.txt", "test"), Response::OK);
    
    EXPECT_EQ(storage.copyFile("/dir1/source.txt", "/dir2/dest.txt"), 
              Response::OK);
    
    std::string content;
    EXPECT_EQ(storage.readFile("/dir2/dest.txt", content), Response::OK);
    EXPECT_EQ(content, "test\n");
}

TEST_F(StorageManagerTest, MoveFile_BothPathsRelative) {
    EXPECT_EQ(storage.makeDir("from"), Response::OK);
    EXPECT_EQ(storage.makeDir("to"), Response::OK);
    EXPECT_EQ(storage.createFile("from/file.txt"), Response::OK);
    
    EXPECT_EQ(storage.moveFile("from/file.txt", "to/file.txt"), Response::OK);
    
    EXPECT_EQ(storage.fileExists("from/file.txt"), Response::NotFound);
    EXPECT_EQ(storage.fileExists("to/file.txt"), Response::OK);
}

TEST_F(StorageManagerTest, MoveFile_WithAbsolutePaths) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("b"), Response::OK);
    EXPECT_EQ(storage.createFile("a/move.txt"), Response::OK);
    EXPECT_EQ(storage.changeDir("a"), Response::OK);
    
    EXPECT_EQ(storage.moveFile("/a/move.txt", "/b/moved.txt"), Response::OK);
    
    EXPECT_EQ(storage.fileExists("/a/move.txt"), Response::NotFound);
    EXPECT_EQ(storage.fileExists("/b/moved.txt"), Response::OK);
}

TEST_F(StorageManagerTest, FileOperations_WithParentDirReferences) {
    EXPECT_EQ(storage.makeDir("parent"), Response::OK);
    EXPECT_EQ(storage.changeDir("parent"), Response::OK);
    EXPECT_EQ(storage.makeDir("child"), Response::OK);
    EXPECT_EQ(storage.changeDir("child"), Response::OK);
    
    EXPECT_EQ(storage.createFile("../file.txt"), Response::OK);
    EXPECT_EQ(storage.fileExists("../file.txt"), Response::OK);
    
    EXPECT_EQ(storage.changeDir(".."), Response::OK);
    EXPECT_EQ(storage.fileExists("file.txt"), Response::OK);
}



// DIRECTORY TESTS
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


TEST_F(StorageManagerTest, MakeAndChangeDir_Success) {
    EXPECT_EQ(storage.makeDir("stuff"), Response::OK);
    EXPECT_EQ(storage.changeDir("stuff"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/stuff");
    EXPECT_EQ(storage.changeDir(".."), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/");
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

TEST_F(StorageManagerTest, ListDir_ShouldReturnFilesAndDirs) {
    storage.createFile("a.txt");
    storage.makeDir("docs");
    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("", list), Response::OK);
    EXPECT_EQ(list.size(), 2);
    EXPECT_TRUE(list[0].rfind("[D]", 0) == 0 || list[1].rfind("[D]", 0) == 0);
    EXPECT_TRUE(list[0].rfind("[F]", 0) == 0 || list[1].rfind("[F]", 0) == 0);
}

TEST_F(StorageManagerTest, CreateFile_ShouldBeListedAfterCreation) {
    EXPECT_EQ(storage.createFile("doc.txt"), Response::OK);
    std::vector<std::string> entries;
    EXPECT_EQ(storage.listDir("", entries), Response::OK);
    ASSERT_EQ(entries.size(), 1);
    EXPECT_TRUE(entries[0].find("doc.txt") != std::string::npos);
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

    std::vector<std::string> before;
    EXPECT_EQ(storage.listDir("", before), Response::OK);
    ASSERT_EQ(before.size(), 1);
    EXPECT_TRUE(before[0].find("parent") != std::string::npos);

    EXPECT_EQ(storage.removeDir("parent"), Response::OK);

    std::vector<std::string> after;
    EXPECT_EQ(storage.listDir("", after), Response::OK);
    EXPECT_TRUE(after.empty());

    EXPECT_EQ(storage.changeDir("parent"), Response::NotFound);
    EXPECT_EQ(storage.changeDir("child"), Response::NotFound);
    EXPECT_EQ(storage.writeFile("rootfile.txt", "hi"), Response::NotFound);
    EXPECT_EQ(storage.writeFile("nested.txt", "hi"), Response::NotFound);
}

TEST_F(StorageManagerTest, ListDir_CurrentDirectory) {
    EXPECT_EQ(storage.makeDir("folder1"), Response::OK);
    EXPECT_EQ(storage.createFile("file1.txt"), Response::OK);
    
    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("", list), Response::OK);
    EXPECT_EQ(list.size(), 2);
    
    int dirs = 0, files = 0;
    for (const auto& entry : list) {
        if (entry.rfind("[D]", 0) == 0) dirs++;
        if (entry.rfind("[F]", 0) == 0) files++;
    }
    EXPECT_EQ(dirs, 1);
    EXPECT_EQ(files, 1);
}

TEST_F(StorageManagerTest, ListDir_WithRelativePath) {
    EXPECT_EQ(storage.makeDir("folder1"), Response::OK);
    EXPECT_EQ(storage.changeDir("folder1"), Response::OK);
    EXPECT_EQ(storage.createFile("file1.txt"), Response::OK);
    EXPECT_EQ(storage.createFile("file2.txt"), Response::OK);
    EXPECT_EQ(storage.changeDir(".."), Response::OK);
    
    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("folder1", list), Response::OK);
    EXPECT_EQ(list.size(), 2);
    
    for (const auto& entry : list) {
        EXPECT_TRUE(entry.rfind("[F]", 0) == 0);
    }
}

TEST_F(StorageManagerTest, ListDir_WithAbsolutePath) {
    EXPECT_EQ(storage.makeDir("folder1"), Response::OK);
    EXPECT_EQ(storage.changeDir("folder1"), Response::OK);
    EXPECT_EQ(storage.makeDir("subfolder"), Response::OK);
    EXPECT_EQ(storage.changeDir("subfolder"), Response::OK);
    EXPECT_EQ(storage.createFile("deep.txt"), Response::OK);
    
    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("/folder1", list), Response::OK);
    EXPECT_EQ(list.size(), 1);
    EXPECT_TRUE(list[0].rfind("[D] subfolder", 0) == 0);
}

TEST_F(StorageManagerTest, ListDir_NestedPath) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.changeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("b"), Response::OK);
    EXPECT_EQ(storage.changeDir("b"), Response::OK);
    EXPECT_EQ(storage.createFile("nested.txt"), Response::OK);
    EXPECT_EQ(storage.changeDir("/"), Response::OK);
    
    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("a/b", list), Response::OK);
    EXPECT_EQ(list.size(), 1);
    EXPECT_TRUE(list[0].rfind("[F] nested.txt", 0) == 0);
}

TEST_F(StorageManagerTest, ListDir_InvalidPath) {
    EXPECT_EQ(storage.makeDir("folder1"), Response::OK);
    
    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("nonexistent", list), Response::NotFound);
    EXPECT_EQ(list.size(), 0);
    
    EXPECT_EQ(storage.listDir("folder1/nonexistent", list), Response::NotFound);
    EXPECT_EQ(list.size(), 0);
}

TEST_F(StorageManagerTest, ListDir_EmptyDirectory) {
    EXPECT_EQ(storage.makeDir("empty"), Response::OK);
    
    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("empty", list), Response::OK);
    EXPECT_EQ(list.size(), 0);
}

TEST_F(StorageManagerTest, ListDir_RootDirectory) {
    EXPECT_EQ(storage.makeDir("folder1"), Response::OK);
    EXPECT_EQ(storage.createFile("file1.txt"), Response::OK);
    EXPECT_EQ(storage.changeDir("folder1"), Response::OK);
    
    std::vector<std::string> list;
    EXPECT_EQ(storage.listDir("/", list), Response::OK);
    EXPECT_EQ(list.size(), 2);
}

TEST_F(StorageManagerTest, ChangeDir_RelativePath) {
    ASSERT_EQ(storage.makeDir("folder1"), Response::OK);
    ASSERT_EQ(storage.changeDir("folder1"), Response::OK);
    ASSERT_EQ(storage.makeDir("folder2"), Response::OK);
    ASSERT_EQ(storage.changeDir(".."), Response::OK);
    
    EXPECT_EQ(storage.changeDir("folder1/folder2"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/folder1/folder2");
}

TEST_F(StorageManagerTest, ChangeDir_AbsolutePath) {
    ASSERT_EQ(storage.makeDir("folder1"), Response::OK);
    ASSERT_EQ(storage.changeDir("folder1"), Response::OK);
    ASSERT_EQ(storage.makeDir("folder2"), Response::OK);
    ASSERT_EQ(storage.makeDir("folder3"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("/folder1/folder2"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/folder1/folder2");
    
    EXPECT_EQ(storage.changeDir("/folder1/folder3"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/folder1/folder3");
}

TEST_F(StorageManagerTest, ChangeDir_ToRoot) {
    ASSERT_EQ(storage.makeDir("folder1"), Response::OK);
    ASSERT_EQ(storage.changeDir("folder1"), Response::OK);
    ASSERT_EQ(storage.makeDir("folder2"), Response::OK);
    ASSERT_EQ(storage.changeDir("folder2"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("/"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/");
}

TEST_F(StorageManagerTest, ChangeDir_ComplexPath) {
    ASSERT_EQ(storage.makeDir("a"), Response::OK);
    ASSERT_EQ(storage.makeDir("b"), Response::OK);
    ASSERT_EQ(storage.changeDir("a"), Response::OK);
    ASSERT_EQ(storage.makeDir("a1"), Response::OK);
    ASSERT_EQ(storage.changeDir(".."), Response::OK);
    ASSERT_EQ(storage.changeDir("b"), Response::OK);
    ASSERT_EQ(storage.makeDir("b1"), Response::OK);
    ASSERT_EQ(storage.changeDir("b1"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("../.."), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/");
    
    EXPECT_EQ(storage.changeDir("a"), Response::OK);
    EXPECT_EQ(storage.changeDir("/b/b1"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/b/b1");
}

TEST_F(StorageManagerTest, ChangeDir_DotCurrent) {
    ASSERT_EQ(storage.makeDir("folder1"), Response::OK);
    ASSERT_EQ(storage.changeDir("folder1"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("."), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/folder1");
    
    ASSERT_EQ(storage.makeDir("folder2"), Response::OK);
    EXPECT_EQ(storage.changeDir("./folder2"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/folder1/folder2");
}

TEST_F(StorageManagerTest, ChangeDir_ParentFromRoot) {
    EXPECT_EQ(storage.changeDir(".."), Response::AtRoot);
    EXPECT_EQ(storage.getWorkingDir(), "/");
}

TEST_F(StorageManagerTest, ChangeDir_InvalidPath) {
    ASSERT_EQ(storage.makeDir("folder1"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("nonexistent"), Response::NotFound);
    EXPECT_EQ(storage.changeDir("folder1/nonexistent"), Response::NotFound);
    EXPECT_EQ(storage.changeDir("/nonexistent/folder"), Response::NotFound);
    
    EXPECT_EQ(storage.changeDir(""), Response::InvalidArgument);
    
    EXPECT_EQ(storage.changeDir("   "), Response::InvalidArgument);
}

TEST_F(StorageManagerTest, ChangeDir_DeepNesting) {
    // Setup: create deep structure
    ASSERT_EQ(storage.makeDir("a"), Response::OK);
    ASSERT_EQ(storage.changeDir("a"), Response::OK);
    ASSERT_EQ(storage.makeDir("b"), Response::OK);
    ASSERT_EQ(storage.changeDir("b"), Response::OK);
    ASSERT_EQ(storage.makeDir("c"), Response::OK);
    ASSERT_EQ(storage.changeDir("c"), Response::OK);
    ASSERT_EQ(storage.makeDir("d"), Response::OK);
    ASSERT_EQ(storage.changeDir(".."), Response::OK);
    ASSERT_EQ(storage.changeDir(".."), Response::OK);
    ASSERT_EQ(storage.changeDir(".."), Response::OK);
    
    EXPECT_EQ(storage.changeDir("a/b/c/d"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/a/b/c/d");
    
    EXPECT_EQ(storage.changeDir("../../.."), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/a");
}

TEST_F(StorageManagerTest, ChangeDir_MultipleSlashes) {
    ASSERT_EQ(storage.makeDir("folder1"), Response::OK);
    ASSERT_EQ(storage.changeDir("folder1"), Response::OK);
    ASSERT_EQ(storage.makeDir("folder2"), Response::OK);
    ASSERT_EQ(storage.changeDir(".."), Response::OK);
    
    EXPECT_EQ(storage.changeDir("folder1//folder2"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/folder1/folder2");
    
    EXPECT_EQ(storage.changeDir("///"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/");
}

TEST_F(StorageManagerTest, ChangeDir_TrailingSlash) {
    ASSERT_EQ(storage.makeDir("folder1"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("folder1/"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/folder1");
}



// DIRECTORY TESTS WITH PATHS
TEST_F(StorageManagerTest, MakeDir_WithRelativePath) {
    EXPECT_EQ(storage.makeDir("parent"), Response::OK);
    EXPECT_EQ(storage.makeDir("parent/child"), Response::OK);
    EXPECT_EQ(storage.changeDir("parent/child"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/parent/child");
}

TEST_F(StorageManagerTest, MakeDir_WithDeepRelativePath) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.changeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("b"), Response::OK);
    EXPECT_EQ(storage.changeDir(".."), Response::OK);
    
    EXPECT_EQ(storage.makeDir("a/b/c"), Response::OK);
    EXPECT_EQ(storage.changeDir("a/b/c"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/a/b/c");
}

TEST_F(StorageManagerTest, MakeDir_WithAbsolutePath) {
    EXPECT_EQ(storage.makeDir("folder"), Response::OK);
    EXPECT_EQ(storage.changeDir("folder"), Response::OK);
    
    EXPECT_EQ(storage.makeDir("/newdir"), Response::OK);
    EXPECT_EQ(storage.changeDir("/newdir"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/newdir");
}

TEST_F(StorageManagerTest, RemoveDir_WithRelativePath) {
    EXPECT_EQ(storage.makeDir("parent"), Response::OK);
    EXPECT_EQ(storage.makeDir("parent/child"), Response::OK);
    EXPECT_EQ(storage.removeDir("parent/child"), Response::OK);
    EXPECT_EQ(storage.changeDir("parent/child"), Response::NotFound);
}

TEST_F(StorageManagerTest, RemoveDir_WithAbsolutePath) {
    EXPECT_EQ(storage.makeDir("dir1"), Response::OK);
    EXPECT_EQ(storage.makeDir("dir1/dir2"), Response::OK);
    EXPECT_EQ(storage.changeDir("dir1"), Response::OK);
    
    EXPECT_EQ(storage.removeDir("/dir1/dir2"), Response::OK);
    EXPECT_EQ(storage.changeDir("dir2"), Response::NotFound);
}

TEST_F(StorageManagerTest, ChangeDir_WithRelativePath) {
    EXPECT_EQ(storage.makeDir("level1"), Response::OK);
    EXPECT_EQ(storage.changeDir("level1"), Response::OK);
    EXPECT_EQ(storage.makeDir("level2"), Response::OK);
    EXPECT_EQ(storage.changeDir(".."), Response::OK);
    
    EXPECT_EQ(storage.changeDir("level1/level2"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/level1/level2");
}

TEST_F(StorageManagerTest, ChangeDir_WithAbsolutePath) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/b"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/b/c"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("/a/b/c"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/a/b/c");
}

TEST_F(StorageManagerTest, ChangeDir_WithParentReferences) {
    EXPECT_EQ(storage.makeDir("a"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/b"), Response::OK);
    EXPECT_EQ(storage.makeDir("a/c"), Response::OK);
    EXPECT_EQ(storage.changeDir("a/b"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("../c"), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/a/c");
}

TEST_F(StorageManagerTest, ChangeDir_WithCurrentDirReference) {
    EXPECT_EQ(storage.makeDir("test"), Response::OK);
    EXPECT_EQ(storage.changeDir("test"), Response::OK);
    EXPECT_EQ(storage.changeDir("."), Response::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/test");
}

TEST_F(StorageManagerTest, CopyDir_BothPathsRelative) {
    EXPECT_EQ(storage.makeDir("original"), Response::OK);
    EXPECT_EQ(storage.makeDir("target"), Response::OK);
    EXPECT_EQ(storage.createFile("original/file.txt"), Response::OK);
    
    EXPECT_EQ(storage.copyDir("original", "target/copy"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("target/copy"), Response::OK);
    EXPECT_EQ(storage.fileExists("file.txt"), Response::OK);
}

TEST_F(StorageManagerTest, CopyDir_BothPathsAbsolute) {
    EXPECT_EQ(storage.makeDir("src"), Response::OK);
    EXPECT_EQ(storage.makeDir("dest"), Response::OK);
    EXPECT_EQ(storage.createFile("src/data.txt"), Response::OK);
    EXPECT_EQ(storage.writeFile("src/data.txt", "content"), Response::OK);
    
    EXPECT_EQ(storage.copyDir("/src", "/dest/backup"), Response::OK);
    
    std::string content;
    EXPECT_EQ(storage.readFile("/dest/backup/data.txt", content), Response::OK);
    EXPECT_EQ(content, "content\n");
}

TEST_F(StorageManagerTest, CopyDir_SourceRelativeDestAbsolute) {
    EXPECT_EQ(storage.makeDir("projects"), Response::OK);
    EXPECT_EQ(storage.changeDir("projects"), Response::OK);
    EXPECT_EQ(storage.makeDir("myapp"), Response::OK);
    EXPECT_EQ(storage.createFile("myapp/readme.txt"), Response::OK);
    
    EXPECT_EQ(storage.copyDir("myapp", "/backup"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("/backup"), Response::OK);
    EXPECT_EQ(storage.fileExists("readme.txt"), Response::OK);
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

TEST_F(StorageManagerTest, MoveDir_BothPathsAbsolute) {
    EXPECT_EQ(storage.makeDir("source"), Response::OK);
    EXPECT_EQ(storage.makeDir("destination"), Response::OK);
    EXPECT_EQ(storage.createFile("source/file.dat"), Response::OK);
    
    EXPECT_EQ(storage.moveDir("/source", "/destination/moved"), Response::OK);
    
    EXPECT_EQ(storage.changeDir("/source"), Response::NotFound);
    EXPECT_EQ(storage.fileExists("/destination/moved/file.dat"), Response::OK);
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

TEST_F(StorageManagerTest, ComplexPath_CreateAndAccessNestedStructure) {
    EXPECT_EQ(storage.makeDir("workspace"), Response::OK);
    EXPECT_EQ(storage.makeDir("workspace/project"), Response::OK);
    EXPECT_EQ(storage.makeDir("workspace/project/src"), Response::OK);
    EXPECT_EQ(storage.createFile("workspace/project/src/main.cpp"), Response::OK);
    EXPECT_EQ(storage.writeFile("workspace/project/src/main.cpp", "int main()"), 
              Response::OK);
    
    EXPECT_EQ(storage.changeDir("workspace"), Response::OK);
    std::string content;
    EXPECT_EQ(storage.readFile("project/src/main.cpp", content), Response::OK);
    EXPECT_EQ(content, "int main()\n");
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

TEST_F(StorageManagerTest, PathOperations_InvalidPathReturnsNotFound) {
    EXPECT_EQ(storage.createFile("nonexistent/file.txt"), Response::NotFound);
    EXPECT_EQ(storage.makeDir("missing/dir"), Response::NotFound);
    EXPECT_EQ(storage.writeFile("ghost/path/file.txt", "data"), 
              Response::NotFound);
}

TEST_F(StorageManagerTest, PathOperations_EmptyPathReturnsInvalidArgument) {
    EXPECT_EQ(storage.createFile(""), Response::InvalidArgument);
    EXPECT_EQ(storage.makeDir(""), Response::InvalidArgument);
    EXPECT_EQ(storage.deleteFile(""), Response::InvalidArgument);
    EXPECT_EQ(storage.removeDir(""), Response::InvalidArgument);
}