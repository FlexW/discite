namespace Dc
{
    public abstract class Entity
    {
        protected Entity()
        {
            id = 0;
        }

        internal Entity(ulong id)
        {
            this.id = id;
        }

        public ulong id { get; private set;  }

        public abstract void OnUpdate(float delta_time);
    };
}
