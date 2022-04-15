using System.Runtime.CompilerServices;

namespace Dc
{
    public static class Scene
    {
        public static Entity CreateEntity(string name = "New entity") => new Entity(CreateEntity_Native(name));
        public static void RemoveEntity(Entity entity) => RemoveEntity_Native(entity.Id);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ulong CreateEntity_Native(string name);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void RemoveEntity_Native(ulong entityId);
    }
}
