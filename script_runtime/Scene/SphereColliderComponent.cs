using System.Runtime.CompilerServices;

namespace Dc
{
    public class SphereColliderComponent : Component
    {
        public float Radius
        {
            get => GetRadius_Native(Entity.Id);
            set => SetRadius_Native(Entity.Id, value);
        }

        public Vector3 Offset
        {
            get
            {
                GetOffset_Native(Entity.Id, out Vector3 offset);
                return offset;
            }

            set => SetOffset_Native(Entity.Id, ref value);
        }

        public bool IsTrigger
        {
            get => IsTrigger_Native(Entity.Id);
            set => SetTrigger_Native(Entity.Id, value);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float GetRadius_Native(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetRadius_Native(ulong entityID, float radius);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetOffset_Native(ulong entityID, out Vector3 offset);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetOffset_Native(ulong entityID, ref Vector3 offset);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsTrigger_Native(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetTrigger_Native(ulong entityID, bool trigger);
    }
}
