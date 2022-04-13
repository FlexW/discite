namespace Dc
{
    public class Script : Entity
    {
        public void OnCreate()
        {
            Log.Info("On create script entity");
            CollisionBeginEvent += OnCollisionBegin;
            CollisionEndEvent += OnCollisionEnd;
            TriggerBeginEvent += OnTriggerBegin;
            TriggerEndEvent += OnTriggerEnd;
        }

        public void OnUpdate(float delta_time)
        {
            if (Input.IsKeyPressed(Key.Space))
            {
                Log.Info("Space pressed");
            }
        }

        public void OnCollisionBegin(Entity other)
        {
            Log.Info("Collison begin");
        }

        public void OnCollisionEnd(Entity other)
        {
            Log.Info("Collison end");
        }

        public void OnTriggerBegin(Entity other)
        {
            Log.Info("Trigger begin");
        }

        public void OnTriggerEnd(Entity other)
        {
            Log.Info("Trigger end");
        }
    }
}
