using System.Runtime.CompilerServices;

namespace Dc
{
    public static class Log
    {
        internal enum LogLevel
        {
            Debug = (1 << 0),
            Info = (1 << 1),
            Warn = (1 << 2),
            Error = (1 << 3)
        }

        public static void Debug(string format, params object[] parameters)
        {
            LogMessage_Native(LogLevel.Debug, string.Format(format, parameters));
        }

        public static void Info(string format, params object[] parameters)
        {
            LogMessage_Native(LogLevel.Info, string.Format(format, parameters));
        }

        public static void Warn(string format, params object[] parameters)
        {
            LogMessage_Native(LogLevel.Warn, string.Format(format, parameters));
        }

        public static void Error(string format, params object[] parameters)
        {
            LogMessage_Native(LogLevel.Error, string.Format(format, parameters));
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void LogMessage_Native(LogLevel level, string formattedMessage);
    }
}
