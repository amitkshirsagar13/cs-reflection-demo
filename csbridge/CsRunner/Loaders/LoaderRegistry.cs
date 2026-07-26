using System.Reflection;
using CsRunner.Models;

namespace CsRunner.Loaders;

/// <summary>
/// Uses reflection to discover all <see cref="IProfileLoader"/> implementations
/// in the current assembly, instantiate them, and dispatch to the right one
/// based on file extension.
///
/// To add a new format (e.g. CSV), simply implement IProfileLoader in this
/// assembly — the registry picks it up automatically at runtime.
/// </summary>
public sealed class LoaderRegistry
{
    private readonly Dictionary<string, IProfileLoader> _loaders = new(StringComparer.OrdinalIgnoreCase);

    public LoaderRegistry()
    {
        DiscoverLoaders(Assembly.GetExecutingAssembly());
    }

    /// <summary>
    /// Scan the given assembly for concrete <see cref="IProfileLoader"/> types,
    /// create instances via the default constructor, and register them.
    /// </summary>
    private void DiscoverLoaders(Assembly assembly)
    {
        var loaderType = typeof(IProfileLoader);
        var types = assembly.GetTypes()
                            .Where(t => t.IsClass
                                     && !t.IsAbstract
                                     && loaderType.IsAssignableFrom(t));

        foreach (var type in types)
        {
            try
            {
                // ── Reflection: create instance without knowing the concrete type ──
                var instance = (IProfileLoader)Activator.CreateInstance(type)!;
                foreach (var ext in instance.SupportedExtensions)
                {
                    _loaders[ext] = instance;
                    CsLogger.Debug($"LoaderRegistry: registered {type.Name} for '{ext}'");
                }
            }
            catch (Exception ex)
            {
                CsLogger.Warn($"LoaderRegistry: could not instantiate {type.Name}: {ex.Message}");
            }
        }

        CsLogger.Info($"LoaderRegistry: {_loaders.Count} extension(s) registered");
    }

    public IReadOnlyList<UserProfile> Load(string filePath)
    {
        var ext = Path.GetExtension(filePath);
        if (!_loaders.TryGetValue(ext, out var loader))
        {
            throw new NotSupportedException(
                $"No loader registered for extension '{ext}'. " +
                $"Supported: {string.Join(", ", _loaders.Keys)}");
        }

        CsLogger.Info($"LoaderRegistry: dispatching '{filePath}' to {loader.GetType().Name}");

        // ── Reflection: inspect the loader type at runtime ────────────────
        var loaderType = loader.GetType();
        CsLogger.Debug($"  Loader type:       {loaderType.FullName}");
        CsLogger.Debug($"  Implements:        {string.Join(", ", loaderType.GetInterfaces().Select(i => i.Name))}");

        var loadMethod = loaderType.GetMethod(nameof(IProfileLoader.Load));
        if (loadMethod is not null)
            CsLogger.Debug($"  Load method found: {loadMethod.Name}({string.Join(", ", loadMethod.GetParameters().Select(p => p.ParameterType.Name))})");

        // Invoke through interface (could also invoke via MethodInfo for demo)
        var results = loader.Load(filePath);

        // ── Reflection: create new instances from loaded data ─────────────
        // Demonstrate reflection-based instantiation of UserProfile
        var profileType = typeof(UserProfile);
        var clonedProfiles = new List<UserProfile>(results.Count);

        foreach (var source in results)
        {
            // Create a new instance via reflection (no 'new' keyword used here)
            var newProfile = (UserProfile)Activator.CreateInstance(profileType)!;

            // Set each property via reflection
            foreach (var prop in profileType.GetProperties(BindingFlags.Public | BindingFlags.Instance))
            {
                if (!prop.CanRead || !prop.CanWrite) continue;
                var value = prop.GetValue(source);

                // Deep-clone lists so instances are independent
                if (value is List<string> list)
                    value = new List<string>(list);
                else if (value is Address addr)
                {
                    var newAddr = (Address)Activator.CreateInstance(typeof(Address))!;
                    foreach (var ap in typeof(Address).GetProperties())
                    {
                        if (ap.CanRead && ap.CanWrite)
                            ap.SetValue(newAddr, ap.GetValue(addr));
                    }
                    value = newAddr;
                }

                prop.SetValue(newProfile, value);
            }

            clonedProfiles.Add(newProfile);
            CsLogger.Debug($"  Reflection-cloned: id={newProfile.Id}");
        }

        return clonedProfiles;
    }

    public IReadOnlyCollection<string> SupportedExtensions => _loaders.Keys;
}