using System;
using System.Runtime.CompilerServices;

namespace Dc
{
    public abstract class Entity
    {
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

        public virtual void OnUpdate(float delta_time) { }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CreateComponent_Native(ulong entityID, Type type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(ulong entityID, Type type);

    };
}
