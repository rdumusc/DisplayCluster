import QtQuick 1.1
import "style.js" as Style

ControlButton {
    image: action.checked ? action.iconChecked : action.icon
    opacity: action.enabled ? Style.buttonsEnabledOpacity : Style.buttonsDisabledOpacity
}
