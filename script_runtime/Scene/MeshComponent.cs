using System.Runtime.CompilerServices;

namespace Dc
{
    public class MeshComponent : Component
    {
        public void SetMesh(string name) => SetMesh_Native(Entity.Id, name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetMesh_Native(ulong entityId, string name);
    }
}
