using System.Runtime.CompilerServices;

namespace Dc
{
    public class ScriptComponent : Component
    {
        public void SetScript(string name) => SetScript_Native(Entity.Id, name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetScript_Native(ulong entityId, string name);
    }
}
