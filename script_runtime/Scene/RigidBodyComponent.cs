using System.Runtime.CompilerServices;

namespace Dc
{
    public enum RigidBodyType
    {
        Static = 0,
        Dynamic
    }

    public class RigidBodyComponent : Component
    {
        public RigidBodyType BodyType => GetBodyType_Native(Entity.Id);

        public bool IsKinematic
        {
            get => IsKinematic_Native(Entity.Id);
            set => SetKinematic_Native(Entity.Id, value);
        }

        public float Mass
        {
            get => GetMass_Native(Entity.Id);
            set => SetMass_Native(Entity.Id, value);
        }

        public bool IsGravityDisabled
        {
            get => IsGravityDisabled_Native(Entity.Id);
            set => SetGravityDisabled_Native(Entity.Id, value);
        }

        public Vector3 LinearVelocity
        {
            get
            {
                GetLinearVelocity_Native(Entity.Id, out Vector3 result);
                return result;
            }
            set => SetLinearVelocity_Native(Entity.Id, ref value);
        }

        public Vector3 AngularVelocity
        {
            get
            {
                GetAngularVelocity_Native(Entity.Id, out Vector3 result);
                return result;
            }
            set => SetAngularVelocity_Native(Entity.Id, ref value);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern RigidBodyType GetBodyType_Native(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsKinematic_Native(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetKinematic_Native(ulong entityID, bool isKinematic);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float GetMass_Native(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetMass_Native(ulong entityID, float mass);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsGravityDisabled_Native(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetGravityDisabled_Native(ulong entityID, bool disabled);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetLinearVelocity_Native(ulong entityId, out Vector3 velocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetLinearVelocity_Native(ulong entityId, ref Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetAngularVelocity_Native(ulong entityId, out Vector3 velocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetAngularVelocity_Native(ulong entityId, ref Vector3 velocity);
    }
}
