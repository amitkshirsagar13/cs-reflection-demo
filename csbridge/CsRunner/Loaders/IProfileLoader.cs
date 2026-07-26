using CsRunner.Models;

namespace CsRunner.Loaders;

/// <summary>
/// Contract that every profile-loading strategy must implement.
/// New loaders (e.g. XML, CSV) are discovered by reflection at runtime.
/// </summary>
public interface IProfileLoader
{
    /// <summary>File extensions this loader handles, e.g. ".json", ".yaml".</summary>
    IReadOnlyList<string> SupportedExtensions { get; }

    /// <summary>Load profiles from the given file path.</summary>
    IReadOnlyList<UserProfile> Load(string filePath);
}