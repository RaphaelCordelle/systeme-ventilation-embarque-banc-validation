from abc import ABC, abstractmethod


class DeviceTransport(ABC):
    """
    Interface commune utilisée par les scénarios.

    Une implémentation peut être :
    - SimulatedTransport : SIL
    - SerialTransport    : HIL
    """

    @abstractmethod
    def connect(self) -> None:
        """Initialise la connexion ou l'environnement."""
        pass

    @abstractmethod
    def close(self) -> None:
        """Ferme proprement la connexion ou l'environnement."""
        pass

    @abstractmethod
    def get_status(self) -> dict:
        """Retourne l'état courant du DUT."""
        pass

    @abstractmethod
    def get_telemetry(self) -> dict:
        """Retourne la télémétrie du DUT."""
        pass

    @abstractmethod
    def open_actuator(self) -> None:
        """Demande l'ouverture."""
        pass

    @abstractmethod
    def close_actuator(self) -> None:
        """Demande la fermeture."""
        pass