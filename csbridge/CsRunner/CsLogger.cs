namespace CsRunner;

/// <summary>
/// Minimal console logger for the C# bridge.
/// Emits ANSI-coloured output so it blends with the C++ terminal style.
/// </summary>
public static class CsLogger
{
    // Sync with C++ conf.yml log level at runtime via env var CSRUNNER_LOG_LEVEL
    private static readonly bool IsDebug =
        string.Equals(
            Environment.GetEnvironmentVariable("CSRUNNER_LOG_LEVEL"),
            "DEBUG",
            StringComparison.OrdinalIgnoreCase);

    public static void Debug(string msg)
    {
        if (!IsDebug) return;
        Console.Error.WriteLine($"\u001b[90m[DEBUG][CsRunner] {msg}\u001b[0m");
    }

    public static void Info(string msg)
    {
        Console.Error.WriteLine($"\u001b[36m[INFO ][CsRunner] {msg}\u001b[0m");
    }

    public static void Warn(string msg)
    {
        Console.Error.WriteLine($"\u001b[33m[WARN ][CsRunner] {msg}\u001b[0m");
    }

    public static void Error(string msg)
    {
        Console.Error.WriteLine($"\u001b[31m[ERROR][CsRunner] {msg}\u001b[0m");
    }
}