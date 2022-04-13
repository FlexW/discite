using System.Runtime.CompilerServices;

namespace Dc
{

    public class Input
    {
        public static bool IsKeyPressed(Key key)
        {
            return IsKeyPressed_Native(key);
        }

        public static bool IsMouseButtonPressed(MouseButton button)
        {
            return IsMouseButtonPressed_Native(button);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsKeyPressed_Native(Key key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsMouseButtonPressed_Native(MouseButton button);
    }
}
