using System;

namespace Dc
{
    class EnemySpawner : Entity
    {
        private int range = 30;
        private Random rnd = new Random();

        void OnUpdate(float deltaTime)
        {
            var randomNumber = rnd.Next(0, 100);
            if (randomNumber == 0)
            {
                var spawnPoint = rnd.Next(-range, range + 1);
                SpawnEnemy(spawnPoint);
            }
        }

        private void SpawnEnemy(float spawnPoint)
        {
            var enemy = Scene.CreateEntity("Enemy");
            enemy.Position = Position + new Vector3(spawnPoint, 1.5f, -60.0f);

            var mesh = enemy.CreateComponent<MeshComponent>();
            mesh.SetMesh("meshes/cube.dcmesh");

            enemy.CreateComponent<BoxColliderComponent>();
            var rigid_body = enemy.CreateComponent<RigidBodyComponent>();
            rigid_body.IsGravityDisabled = true;

            var script = enemy.CreateComponent<ScriptComponent>();
            script.SetScript("Dc.Enemy");
        }
    };
}
