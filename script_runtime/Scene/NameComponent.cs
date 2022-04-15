using System.Runtime.CompilerServices;

namespace Dc
{
    public class NameComponent : Component
    {
        public string Name
        {
            get
            {
                return GetTag_Native(Entity.Id);
            }
            set
            {
                SetTag_Native(Entity.Id, value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetTag_Native(ulong entityId);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTag_Native(ulong entityId, string tag);
    }
}
