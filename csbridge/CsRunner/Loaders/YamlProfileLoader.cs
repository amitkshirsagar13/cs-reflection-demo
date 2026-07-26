using CsRunner.Models;
using YamlDotNet.RepresentationModel;

namespace CsRunner.Loaders;

/// <summary>Loads user profiles from a YAML file using YamlDotNet.</summary>
public sealed class YamlProfileLoader : IProfileLoader
{
    public IReadOnlyList<string> SupportedExtensions { get; } = [".yml", ".yaml"];

    public IReadOnlyList<UserProfile> Load(string filePath)
    {
        CsLogger.Debug($"YamlProfileLoader.Load: {filePath}");

        using var reader = new StreamReader(filePath);
        var yaml = new YamlStream();
        yaml.Load(reader);

        if (yaml.Documents.Count == 0)
            return [];

        var root = yaml.Documents[0].RootNode;

        // Accept: sequence at root, or mapping with key "users" / "profiles"
        YamlSequenceNode sequence;
        if (root is YamlSequenceNode seq)
        {
            sequence = seq;
        }
        else if (root is YamlMappingNode map)
        {
            var key = map.Children.Keys
                         .OfType<YamlScalarNode>()
                         .FirstOrDefault(k => k.Value is "users" or "profiles");
            if (key is null || map[key] is not YamlSequenceNode s)
                throw new InvalidDataException(
                    "YAML must be a sequence or mapping with 'users'/'profiles' sequence");
            sequence = s;
        }
        else
        {
            throw new InvalidDataException("Unexpected YAML root node type");
        }

        var profiles = new List<UserProfile>();
        foreach (var node in sequence)
        {
            if (node is not YamlMappingNode item) continue;
            try
            {
                var p = MapFromNode(item);
                profiles.Add(p);
                CsLogger.Debug($"  Parsed profile id={p.Id} username={p.Username}");
            }
            catch (Exception ex)
            {
                CsLogger.Warn($"  Skipping item: {ex.Message}");
            }
        }

        CsLogger.Info($"YamlProfileLoader: loaded {profiles.Count} profile(s)");
        return profiles;
    }

    // ── Helpers ──────────────────────────────────────────────────────────

    private static string Scalar(YamlMappingNode node, string key)
    {
        var k = node.Children.Keys
                     .OfType<YamlScalarNode>()
                     .FirstOrDefault(n => n.Value == key);
        return (k is not null && node[k] is YamlScalarNode val)
               ? val.Value ?? string.Empty
               : string.Empty;
    }

    private static List<string> Sequence(YamlMappingNode node, string key)
    {
        var k = node.Children.Keys
                     .OfType<YamlScalarNode>()
                     .FirstOrDefault(n => n.Value == key);
        if (k is null || node[k] is not YamlSequenceNode seq) return [];
        return seq.Children
                  .OfType<YamlScalarNode>()
                  .Select(s => s.Value ?? string.Empty)
                  .ToList();
    }

    private static UserProfile MapFromNode(YamlMappingNode item)
    {
        var p = new UserProfile
        {
            Id        = Scalar(item, "id"),
            FirstName = Scalar(item, "firstName"),
            LastName  = Scalar(item, "lastName"),
            Username  = Scalar(item, "username"),
            Password  = Scalar(item, "password"),
            Age       = int.TryParse(Scalar(item, "age"), out int a) ? a : 0,
            Emails    = Sequence(item, "emails"),
            Mobiles   = Sequence(item, "mobiles"),
        };

        // Address
        var addrKey = item.Children.Keys
                          .OfType<YamlScalarNode>()
                          .FirstOrDefault(n => n.Value == "address");
        if (addrKey is not null && item[addrKey] is YamlMappingNode addr)
        {
            p.Address = new Address
            {
                FirstLine = Scalar(addr, "firstLine"),
                AptUnit   = Scalar(addr, "aptUnit"),
                City      = Scalar(addr, "city"),
                State     = Scalar(addr, "state"),
                Zip       = Scalar(addr, "zip"),
            };
        }

        if (string.IsNullOrWhiteSpace(p.Id))
            throw new InvalidDataException("Profile missing 'id'");

        return p;
    }
}