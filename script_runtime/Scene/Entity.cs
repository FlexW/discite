using System;
using System.Runtime.CompilerServices;

namespace Dc
{
    public class Entity
    {
        public event Action<Entity> CollisionBeginEvent;
        public event Action<Entity> CollisionEndEvent;
        public event Action<Entity> TriggerBeginEvent;
        public event Action<Entity> TriggerEndEvent;

        public event Action<Key> KeyPressEvent;
        public event Action<Key> KeyReleaseEvent;

        protected Entity()
        {
            Id = 0;
        }

        internal Entity(ulong id)
        {
            this.Id = id;
        }

        public ulong Id { get; private set; }
        public string Name => GetComponent<NameComponent>().Name;

        public Vector3 Position
        {
            get
            {
                GetPosition_Native(Id, out Vector3 result);
                return result;
            }
            set => SetPosition_Native(Id, ref value);
        }

        public Vector3 Rotation
        {
            get { GetRotation_Native(Id, out Vector3 result); return result; }
            set => SetRotation_Native(Id, ref value);
        }

        public Vector3 Scale
        {
            get { GetScale_Native(Id, out Vector3 result); return result; }
            set => SetScale_Native(Id, ref value);
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            return HasComponent_Native(Id, typeof(T));
        }

        public T CreateComponent<T>() where T : Component, new()
        {
            CreateComponent_Native(Id, typeof(T));
            T component = new T();
            component.Entity = this;
            return component;
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
            {
                T component = new T();
                component.Entity = this;
                return component;
            }
            return null;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CreateComponent_Native(ulong entityID, Type type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(ulong entityID, Type type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetPosition_Native(ulong entityID, out Vector3 positon);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetPosition_Native(ulong entityID, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetRotation_Native(ulong entityID, out Vector3 rotation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetRotation_Native(ulong entityID, ref Vector3 rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetScale_Native(ulong entityID, out Vector3 scale);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetScale_Native(ulong entityID, ref Vector3 scale);

        private void OnCollisionBeginInternal(ulong id) => CollisionBeginEvent?.Invoke(new Entity(id));
        private void OnCollisionEndInternal(ulong id) => CollisionEndEvent?.Invoke(new Entity(id));

        private void OnTriggerBeginInternal(ulong id) => TriggerBeginEvent?.Invoke(new Entity(id));
        private void OnTriggerEndInternal(ulong id) => TriggerEndEvent?.Invoke(new Entity(id));

        private void OnKeyPressInternal(Key key) => KeyPressEvent?.Invoke(key);
        private void OnKeyReleaseInternal(Key key) => KeyReleaseEvent?.Invoke(key);

    };
}
