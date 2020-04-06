import { HartDeviceDto } from './hart-device';

export interface HartMuxDto extends HartDeviceDto {
    devices: HartDeviceDto[]
    ipAddress: string
}

export interface HartGw {
    ip: string
    modules: string[]
    serialNo: number
}

export const hartServerUrl = "http://"+location.host.split(':')[0]+":5900"

export const updateInterval = 5000