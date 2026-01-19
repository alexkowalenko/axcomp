#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "importer.hh"
#include "symboltable.hh"
#include "typetable.hh"

namespace {

class TestImporter : public ax::Importer {
  public:
    explicit TestImporter(ax::ErrorManager &errors) : ax::Importer(errors) {}

    using ax::Importer::read_module;
    using ax::Importer::set_search_path;
};

class TempDir {
  public:
    TempDir()
        : path_{std::filesystem::temp_directory_path() /
                std::filesystem::path("axcomp-importer-test-%%%%-%%%%")} {
        std::filesystem::create_directories(path_);
    }

    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(path_, ec);
    }

    [[nodiscard]] const std::filesystem::path &path() const { return path_; }

  private:
    std::filesystem::path path_;
};

std::filesystem::path write_definition(const std::filesystem::path &dir,
                                       std::string const           &module_name) {
    auto          file_path = dir / (module_name + ".def");
    std::ofstream os(file_path);
    os << "DEFINITION " << module_name << ";\n"
       << "PROCEDURE Foo*;\n"
       << "END " << module_name << ".\n";
    return file_path;
}

} // namespace

TEST(ImporterReadModule, ModuleSymbolsWhenDefinitionExists) {
    TempDir const temp_dir;
    write_definition(temp_dir.path(), "Sample");

    ax::ErrorManager errors;
    TestImporter     importer(errors);
    importer.set_search_path(temp_dir.path().string());

    ax::TypeTable types;
    types.initialise();

    auto result = importer.read_module("Sample", types);
    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->find("Foo"), nullptr);
}

TEST(ImporterReadModule, EmptyWhenDefinitionIsMissing) {
    TempDir const temp_dir;

    ax::ErrorManager errors;
    TestImporter     importer(errors);
    importer.set_search_path(temp_dir.path().string());

    ax::TypeTable types;
    types.initialise();

    auto result = importer.read_module("NonExistent", types);
    EXPECT_FALSE(result.has_value());
}
